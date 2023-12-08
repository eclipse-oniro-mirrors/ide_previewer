/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "StageContext.h"
#include <sstream>
#include <fstream>
#include <cctype>
#include "json/json.h"
#include "JsonReader.h"
#include "FileSystem.h"
#include "TraceTool.h"
#include "PreviewerEngineLog.h"
#include "zlib.h"
#include "contrib/minizip/unzip.h"
using namespace std;

namespace OHOS::Ide {
StageContext& StageContext::GetInstance()
{
    static StageContext instance;
    return instance;
}

const std::optional<std::vector<uint8_t>> StageContext::ReadFileContents(const std::string& filePath) const
{
    if (!FileSystem::IsFileExists(filePath)) {
        ELOG("file %s is not exist.", filePath.c_str());
        return std::nullopt;
    }
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        ELOG("open file %s failed.", filePath.c_str());
        return std::nullopt;
    }
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> data(fileSize);
    if (file.read(reinterpret_cast<char*>(data.data()), fileSize)) {
        return data;
    } else {
        ELOG("read file %s failed.", filePath.c_str());
        return std::nullopt;
    }
}

void StageContext::SetLoaderJsonPath(const std::string& assetPath)
{
    // Concatenate loader.json path
    std::string flag = assetPath.find(".preview") == std::string::npos ? "loader_out" : "assets";
    std::string separator = FileSystem::GetSeparator();
    size_t pos = assetPath.rfind(flag);
    if (pos == std::string::npos) {
        ELOG("assetPath: %s format error.", assetPath.c_str());
        return;
    }
    std::string temp = ReplaceLastStr(assetPath, "ets", "");
    temp = ReplaceLastStr(temp, flag, "loader");
    loaderJsonPath = temp + "loader.json";
    if (!FileSystem::IsFileExists(loaderJsonPath)) {
        ELOG("the loaderJsonPath %s is not exist.", loaderJsonPath.c_str());
        return;
    }
    ILOG("set loaderJsonPath: %s successed.", loaderJsonPath.c_str());
}

void StageContext::GetModulePathMapFromLoaderJson()
{
    if (!FileSystem::IsFileExists(loaderJsonPath)) {
        ELOG("the loaderJsonPath is not exist.");
        return;
    }
    string jsonStr = JsonReader::ReadFile(loaderJsonPath);
    Json::Value rootJson = JsonReader::ParseJsonData(jsonStr);
    if (!rootJson) {
        ELOG("Get loader.json content failed.");
        return;
    }
    if (!rootJson.isMember("modulePathMap")) {
        ELOG("Don't find modulePathMap node in loader.json.");
        return;
    }
    std::unique_ptr<Json::Value> jsonObj = JsonReader::GetObject(rootJson, "modulePathMap");
    for (const auto& key : jsonObj->getMemberNames()) {
        string val = JsonReader::GetString(*jsonObj, key);
        modulePathMap[key] = val;
    }
    std::unique_ptr<Json::Value> jsonObjOhm = JsonReader::GetObject(rootJson, "harNameOhmMap");
    for (const auto& key : jsonObjOhm->getMemberNames()) {
        string val = JsonReader::GetString(*jsonObjOhm, key);
        harNameOhmMap[key] = val;
    }
}

std::string StageContext::GetHspAceModuleBuild(const std::string& hspConfigPath)
{
    if (!FileSystem::IsFileExists(hspConfigPath)) {
        ELOG("hspConfigPath: %s is not exist.", hspConfigPath.c_str());
        return "";
    }
    string jsonStr = JsonReader::ReadFile(hspConfigPath);
    Json::Value rootJson = JsonReader::ParseJsonData(jsonStr);
    if (!rootJson) {
        ELOG("Get hsp buildConfig.json content failed.");
        return "";
    }
    if (!rootJson.isMember("aceModuleBuild")) {
        ELOG("Don't find aceModuleBuild node in hsp buildConfig.json.");
        return "";
    }
    return JsonReader::GetString(rootJson, "aceModuleBuild");
}

void StageContext::ReleaseHspBuffers()
{
    for (std::vector<uint8_t>* ptr : hspBufferPtrsVec) {
        delete ptr;
    }
    hspBufferPtrsVec.clear();
    ILOG("ReleaseHspBuffers finished.");
}

std::map<std::string, std::string> StageContext::GetModulePathMap() const
{
    return modulePathMap;
}

std::vector<uint8_t>* StageContext::GetModuleBuffer(const std::string& inputPath)
{
    ILOG("inputPath is:%s.", inputPath.c_str());
    TraceTool::GetInstance().HandleTrace("HSP is loaded");
    std::string spliter = "/";
    size_t pos = inputPath.rfind(spliter);
    if (pos == std::string::npos) {
        ELOG("inputPath: %s format error.", inputPath.c_str());
        return nullptr;
    }
    std::string bundleName = inputPath.substr(0, pos);
    ILOG("bundleName is:%s.", bundleName.c_str());
    if (bundleName.empty()) {
        ELOG("bundleName is empty.");
        return nullptr;
    }
    std::string moduleName = inputPath.substr(pos + spliter.size());
    ILOG("moduleName is:%s.", moduleName.c_str());
    if (modulePathMap.empty()) {
        ELOG("modulePathMap is empty.");
        return nullptr;
    }
    if (bundleName == localBundleName) { // local hsp
        if (modulePathMap.count(moduleName) > 0) { // exist local hsp
            return GetLocalModuleBuffer(moduleName);
        } else { // local hsp not exist, load cloud hsp
            ILOG("cloud hsp bundleName is same as the local project.");
            std::string actualName;
            int ret = GetHspActualName(moduleName, actualName);
            if (ret > 1) {
                WLOG("have more same module name hsp in the project, load the first as default.");
            }
            if (actualName.empty()) {
                ELOG("get hsp actual name failed.");
                return nullptr;
            }
            return GetCloudModuleBuffer(actualName);
        }
    } else { // cloud hsp
        return GetCloudModuleBuffer(moduleName);
    }
}

std::vector<uint8_t>* StageContext::GetLocalModuleBuffer(const std::string& moduleName)
{
    std::string modulePath = StageContext::GetInstance().modulePathMap[moduleName];
    if (modulePath.empty()) {
        ELOG("modulePath is empty.");
        return nullptr;
    }
    ILOG("get modulePath: %s successed.", modulePath.c_str());
    if (!FileSystem::IsDirectoryExists(modulePath)) {
        ELOG("don't find moduleName: %s in modulePathMap from loader.json.", moduleName.c_str());
        return nullptr;
    }
    if (ContainsRelativePath(modulePath)) {
        ELOG("modulePath format error: %s.", modulePath.c_str());
        return nullptr;
    }
    std::string separator = FileSystem::GetSeparator();
    // 读取hsp的.preview/config/buildConfig.json获取aceModuleBuild值就是hsp的modules.abc所在文件夹
    std::string hspConfigPath = modulePath + separator + ".preview" + separator + "config" +
        separator + "buildConfig.json";
    std::string abcDir = GetHspAceModuleBuild(hspConfigPath);
    if (!FileSystem::IsDirectoryExists(abcDir)) {
        ELOG("the abcDir:%s is not exist.", abcDir.c_str());
        return nullptr;
    }
    std::string abcPath = abcDir + separator + "modules.abc";
    if (!FileSystem::IsFileExists(abcPath)) {
        ELOG("the abcPath:%s is not exist.", abcPath.c_str());
        return nullptr;
    }
    ILOG("get modules.abc path: %s successed.", abcPath.c_str());
    std::optional<std::vector<uint8_t>> opt = ReadFileContents(abcPath);
    if (!opt.has_value()) {
        ELOG("read modules.abc buffer failed.");
        return nullptr;
    }
    std::vector<uint8_t> *buf = new std::vector<uint8_t>(opt.value());
    hspBufferPtrsVec.push_back(buf);
    return buf;
}

std::vector<uint8_t>* StageContext::GetCloudModuleBuffer(const std::string& moduleName)
{
    // 1.以entry拆分，拼接oh_modules/.hsp,在这个拼接目录下查找以moduleName@开头的文件夹
    // 2.获取拼接目录下的moduleName.hsp文件
    // 3.使用zlib获取hsp压缩包下的ets/modules.abc内容
    int upwardLevel = 5;
    int pos = GetUpwardDirIndex(loaderJsonPath, upwardLevel);
    if (pos < 0) {
        ILOG("GetUpwardDirIndex:%d failed.", pos);
    }
    std::string entryPath = loaderJsonPath.substr(0, pos);
    ILOG("get entryPath:%s", entryPath.c_str());
    std::string hspDir = entryPath + "/oh_modules/.hsp";
    if (!FileSystem::IsDirectoryExists(hspDir)) {
        ELOG("hspDir: %s is not exist.", hspDir.c_str());
        return nullptr;
    }
    std::string hspPath = GetCloudHspPath(hspDir, moduleName);
    ILOG("get hspPath:%s moduleName:%s", hspPath.c_str(), moduleName.c_str());
    if (!FileSystem::IsDirectoryExists(hspPath)) {
        ELOG("hspPath: %s is not exist.", hspPath.c_str());
        return nullptr;
    }
    std::string moduleHspFile = hspPath + "/" + moduleName + ".hsp";
    ILOG("get moduleHspFile:%s.", moduleHspFile.c_str());
    if (!FileSystem::IsFileExists(moduleHspFile)) {
        ELOG("the moduleHspFile:%s is not exist.", moduleHspFile.c_str());
        return nullptr;
    }
    // unzip and get ets/moudles.abc buffer
    std::vector<uint8_t>* buf = GetModuleBufferFromHsp(moduleHspFile, "ets/modules.abc");
    if (!buf) {
        ELOG("read modules.abc buffer failed.");
    }
    return buf;
}

std::string StageContext::GetCloudHspPath(const std::string& hspDir, const std::string& moduleName)
{
    string flag = "@";
    std::string partName = moduleName + flag;
    return FileSystem::FindSubfolderByName(hspDir, partName);
}

std::vector<uint8_t>* StageContext::GetModuleBufferFromHsp(const std::string& hspFilePath,
    const std::string& fileName)
{
    unzFile zipfile = unzOpen2(hspFilePath.c_str(), nullptr);
    if (zipfile == NULL) {
        printf("Failed to open the zip file: %s\n", hspFilePath.c_str());
        return nullptr;
    }

    if (unzLocateFile(zipfile, fileName.c_str(), 1) != UNZ_OK) {
        printf("Failed to locate the file: %s\n", fileName.c_str());
        unzClose(zipfile);
        return nullptr;
    }

    unz_file_info file_info;
    if (unzGetCurrentFileInfo(zipfile, &file_info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) {
        printf("Failed to get the file info: %s\n", fileName.c_str());
        unzClose(zipfile);
        return nullptr;
    }

    if (unzOpenCurrentFile(zipfile) != UNZ_OK) {
        printf("Failed to open the file: %s\n", fileName.c_str());
        unzClose(zipfile);
        return nullptr;
    }

    char buffer[1024];
    int bytesRead;
    std::vector<uint8_t>* fileContent = new std::vector<uint8_t>();
    while ((bytesRead = unzReadCurrentFile(zipfile, buffer, sizeof(buffer))) > 0) {
        fileContent->insert(fileContent->end(), buffer, buffer + bytesRead);
    }
    hspBufferPtrsVec.push_back(fileContent);
    unzCloseCurrentFile(zipfile);
    unzClose(zipfile);

    printf("File extracted and content saved: %s\n", fileName.c_str());
    return fileContent;
}

bool StageContext::ContainsRelativePath(const std::string& path) const
{
    std::string flg1 = ".." + FileSystem::GetSeparator();
    std::string flg2 = "." + FileSystem::GetSeparator();
    return (path.find(flg1) != std::string::npos || path.find(flg2) != std::string::npos);
}

std::map<string, string> StageContext::ParseMockJsonFile(const std::string& mockJsonFilePath)
{
    std::map<string, string> mapInfo;
    if (!FileSystem::IsFileExists(mockJsonFilePath)) {
        ELOG("the mockJsonFilePath:%s is not exist.", mockJsonFilePath.c_str());
        return mapInfo;
    }
    std::string jsonStr = JsonReader::ReadFile(mockJsonFilePath);
    Json::Value rootJson = JsonReader::ParseJsonData(jsonStr);
    if (!rootJson) {
        ELOG("get mock-config.json content failed.");
        return mapInfo;
    }
    for (const auto& key : rootJson.getMemberNames()) {
        if (!rootJson[key].isNull() && rootJson[key].isMember("source") && rootJson[key]["source"].isString()) {
            mapInfo[key] = rootJson[key]["source"].asString();
        }
    }
    return mapInfo;
}

int StageContext::GetUpwardDirIndex(const std::string& path, const int upwardLevel) const
{
    std::string::size_type pos = path.find_last_of(FileSystem::GetSeparator().c_str());
    std::string::size_type count = 0;
    while (count < upwardLevel) {
        if (pos == std::string::npos) {
            ELOG("GetUpwardDir:%s failed.");
            int errCode = -1;
            return errCode;
        }
        pos = path.find_last_of(FileSystem::GetSeparator().c_str(), pos - 1);
        ++count;
    }
    ILOG("GetUpwardDir path:%s pos:%d", path.c_str(), pos);
    return pos;
}

std::string StageContext::ReplaceLastStr(const std::string& str, const std::string& find, const std::string& replace)
{
    std::string ret = str;
    size_t pos = ret.rfind(find);
    if (pos != std::string::npos) {
        ret.replace(pos, find.size(), replace);
    }
    return ret;
}

int StageContext::GetHspActualName(const std::string& input, std::string& ret)
{
    int num = 0;
    string flag = "/" + input + "/";
    for (const auto& pair : harNameOhmMap) {
        if (pair.second.find(flag) != std::string::npos) {
            if (num == 0) {
                ret = pair.first;
            }
            num++;
            WLOG("find hsp actual name:%s", pair.first.c_str());
        }
    }
    return num;
}
}