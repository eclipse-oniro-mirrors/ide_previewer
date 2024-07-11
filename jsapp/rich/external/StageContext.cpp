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
#include "JsonReader.h"
#include "FileSystem.h"
#include "TraceTool.h"
#include "PreviewerEngineLog.h"
#include "CommandParser.h"
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
    loaderJsonPath = FileSystem::NormalizePath(assetPath);
    if (loaderJsonPath.empty() || !FileSystem::IsFileExists(loaderJsonPath)) {
        ELOG("the loaderJsonPath %s is not exist.", loaderJsonPath.c_str());
        return;
    }
    ILOG("set loaderJsonPath: %s successed.", loaderJsonPath.c_str());
}

void StageContext::SetHosSdkPath(const std::string& hosSdkPathValue)
{
    this->hosSdkPath = hosSdkPathValue;
}

void StageContext::GetModulePathMapFromLoaderJson()
{
    if (!FileSystem::IsFileExists(loaderJsonPath)) {
        ELOG("the loaderJsonPath is not exist.");
        return;
    }
    string jsonStr = JsonReader::ReadFile(loaderJsonPath);
    Json2::Value rootJson = JsonReader::ParseJsonData2(jsonStr);
    if (rootJson.IsNull() || !rootJson.IsValid()) {
        ELOG("Get loader.json content failed.");
        return;
    }
    if (!rootJson.IsMember("modulePathMap") || !rootJson.IsMember("harNameOhmMap") ||
        !rootJson.IsMember("projectRootPath")) {
        ELOG("Don't find some necessary node in loader.json.");
        return;
    }
    Json2::Value jsonObj = rootJson["modulePathMap"];
    for (const auto& key : jsonObj.GetMemberNames()) {
        modulePathMap[key] = jsonObj[key].AsString();
    }
    Json2::Value jsonObjOhm = rootJson["hspNameOhmMap"];
    if (jsonObjOhm.IsNull() || !jsonObjOhm.IsValid()) {
        ILOG("hspNameOhmMap isNull");
        jsonObjOhm = rootJson["harNameOhmMap"];
    }
    for (const auto& key : jsonObjOhm.GetMemberNames()) {
        hspNameOhmMap[key] = jsonObjOhm[key].AsString();
    }
    projectRootPath = rootJson["projectRootPath"].AsString();
    if (rootJson.IsMember("buildConfigPath")) {
        buildConfigPath = rootJson["buildConfigPath"].AsString();
    }
}

std::string StageContext::GetHspAceModuleBuild(const std::string& hspConfigPath)
{
    if (!FileSystem::IsFileExists(hspConfigPath)) {
        ELOG("hspConfigPath: %s is not exist.", hspConfigPath.c_str());
        return "";
    }
    string jsonStr = JsonReader::ReadFile(hspConfigPath);
    Json2::Value rootJson = JsonReader::ParseJsonData2(jsonStr);
    if (rootJson.IsNull() || !rootJson.IsValid()) {
        ELOG("Get hsp buildConfig.json content failed.");
        return "";
    }
    if (!rootJson.IsMember("aceModuleBuild")) {
        ELOG("Don't find aceModuleBuild node in hsp buildConfig.json.");
        return "";
    }
    return rootJson["aceModuleBuild"].AsString();
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
            return GetCloudModuleBuffer(moduleName);
        }
    } else {
        // 先找三方hsp，再找系统hsp
        std::vector<uint8_t>* buf = GetCloudModuleBuffer(moduleName);
        if (buf) { // cloud hsp
            return buf;
        } else { // system hsp
            std::vector<uint8_t>* buf = GetSystemModuleBuffer(inputPath, moduleName);
            ILOG("system hsp buf size is %d", buf->size());
            return buf;
        }
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
    if (!buildConfigPath.empty()) {
        ILOG("buildConfigPath is not empty.");
        hspConfigPath = modulePath + separator + buildConfigPath;
    }
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

std::string StageContext::GetCloudHspVersion(const std::string& hspPath, const std::string& actualName)
{
    string flag = "@";
    std::string spliter = actualName + flag;
    // 以partName字符串拆分出版本号
    size_t pos = hspPath.rfind(spliter);
    if (pos == std::string::npos) {
        ELOG("hspPath: %s format error. no spliter:%s exist", hspPath.c_str(), spliter.c_str());
        return "";
    }
    int idx = pos + spliter.size();
    return hspPath.substr(idx);
}

std::vector<int> StageContext::SplitHspVersion(const std::string& version)
{
    std::vector<int> segments;
    std::istringstream iss(version);
    std::string segment;
    while (getline(iss, segment, '.')) {
        segments.push_back(std::stoi(segment));
    }
    return segments;
}

int StageContext::CompareHspVersion(const std::string& version1, const std::string& version2)
{
    ILOG("module hsp version:%s, project hsp version:%s", version1.c_str(), version2.c_str());
    std::vector<int> ver1 = SplitHspVersion(version1);
    std::vector<int> ver2 = SplitHspVersion(version2);
    // 将两个版本号的分段个数补齐
    while (ver1.size() < ver2.size()) {
        ver1.push_back(0);
    }
    while (ver2.size() < ver1.size()) {
        ver2.push_back(0);
    }
    // 逐段比较版本号
    for (size_t i = 0; i < ver1.size(); ++i) {
        if (ver1[i] < ver2[i]) {
            return -1;
        } else if (ver1[i] > ver2[i]) {
            return 1;
        }
    }
    return 0;
}

std::string StageContext::GetActualCloudHspDir(const std::string& actualName)
{
    string moduleHspPath = GetCloudModuleHspPath(actualName);
    string projectHspPath = GetCloudProjectHspPath(actualName);
    ILOG("moduleHspPath:%s, projectHspPath:%s", moduleHspPath.c_str(), projectHspPath.c_str());
    if (moduleHspPath.empty() || !FileSystem::IsDirectoryExists(moduleHspPath)) {
        return projectHspPath; // 模块级不存在，加载项目级
    }
    if (projectHspPath.empty() || !FileSystem::IsDirectoryExists(projectHspPath)) {
        return moduleHspPath; // 模块级存在，项目级不存在，加载模块级
    }
    // 模块级和项目级都存在，加载版本号高的
    string moduleHspVersion = GetCloudHspVersion(moduleHspPath, actualName);
    string projectHspVersion = GetCloudHspVersion(projectHspPath, actualName);
    if (moduleHspVersion.empty()) {
        return projectHspPath; // 模块级版本号不存在，加载项目级
    }
    if (projectHspVersion.empty()) {
        return moduleHspPath; // 模块级版本号存在，项目级版本号不存在，加载模块级
    }
    int ret = CompareHspVersion(moduleHspVersion, projectHspVersion);
    ILOG("CompareHspVersion result is:%d", ret);
    return ret >= 0 ? moduleHspPath : projectHspPath; // 优先加载版本号高的，版本号相同则优先加载模块级的
}

std::string StageContext::GetCloudProjectHspPath(const std::string& actualName)
{
    ILOG("get projectRootPath:%s", projectRootPath.c_str());
    std::string hspDir = projectRootPath + "/oh_modules/.hsp";
    if (!FileSystem::IsDirectoryExists(hspDir)) {
        ELOG("hspDir: %s in project is not exist.", hspDir.c_str());
        return "";
    }
    return GetCloudHspPath(hspDir, actualName);
}

std::string StageContext::GetCloudModuleHspPath(const std::string& actualName)
{
    int upwardLevel = 5;
    int pos = GetUpwardDirIndex(loaderJsonPath, upwardLevel);
    if (pos < 0) {
        ILOG("GetUpwardDirIndex:%d failed.", pos);
        return "";
    }
    std::string moduleRootPath = loaderJsonPath.substr(0, pos);
    ILOG("get moduleRootPath:%s", moduleRootPath.c_str());
    std::string hspDir = moduleRootPath + "/oh_modules/.hsp";
    if (!FileSystem::IsDirectoryExists(hspDir)) {
        ELOG("hspDir: %s in module is not exist.", hspDir.c_str());
        return "";
    }
    return GetCloudHspPath(hspDir, actualName);
}

std::vector<uint8_t>* StageContext::GetCloudModuleBuffer(const std::string& moduleName)
{
    std::string actualName;
    int ret = GetHspActualName(moduleName, actualName);
    if (ret > 1) {
        WLOG("have more same module name hsp in the project, load the first as default.");
    }
    if (actualName.empty()) {
        ELOG("get hsp actual name failed.");
        return nullptr;
    }
    // 1.以entry(指代模块根目录或项目根目录)拆分，拼接oh_modules/.hsp,在这个拼接目录下查找以actualName@开头的文件夹
    // 2.获取拼接目录下的actualName.hsp文件
    // 3.使用zlib获取hsp压缩包下的ets/modules.abc内容
    std::string hspPath = GetActualCloudHspDir(actualName);
    ILOG("get hspPath:%s actualName:%s", hspPath.c_str(), actualName.c_str());
    if (!FileSystem::IsDirectoryExists(hspPath)) {
        ELOG("hspPath: %s is not exist.", hspPath.c_str());
        return nullptr;
    }
    std::string moduleHspFile = hspPath + "/" + actualName + ".hsp";
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
    Json2::Value rootJson = JsonReader::ParseJsonData2(jsonStr);
    if (rootJson.IsNull() || !rootJson.IsValid()) {
        ELOG("get mock-config.json content failed.");
        return mapInfo;
    }
    for (const auto& key : rootJson.GetMemberNames()) {
        if (!rootJson[key].IsNull() && rootJson[key].IsMember("source") && rootJson[key]["source"].IsString()) {
            mapInfo[key] = rootJson[key]["source"].AsString();
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
    for (const auto& pair : hspNameOhmMap) {
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

std::vector<uint8_t>* StageContext::GetSystemModuleBuffer(const std::string& inputPath,
    const std::string& moduleName)
{
    string head = "com.huawei";
    string tail = moduleName;
    size_t pos1 = inputPath.find(head) + head.size();
    size_t pos2 = inputPath.find(tail);
    std::string relativePath = inputPath.substr(pos1, pos2 - pos1);
    size_t found = relativePath.find(".");
    int len = 1;
    while (found != std::string::npos) {
        relativePath.replace(found, len, "/");
        found = relativePath.find(".", found + len);
    }
    std::string moduleHspFile = hosSdkPath + "/systemHsp" + relativePath + moduleName + ".hsp";
    ILOG("get system moduleHspFile:%s.", moduleHspFile.c_str());
    if (!FileSystem::IsFileExists(moduleHspFile)) {
        ELOG("the system moduleHspFile:%s is not exist.", moduleHspFile.c_str());
        return nullptr;
    }
    // unzip and get ets/moudles.abc buffer
    std::vector<uint8_t>* buf = GetModuleBufferFromHsp(moduleHspFile, "ets/modules.abc");
    if (!buf) {
        ELOG("read modules.abc buffer failed.");
    }
    return buf;
}

void StageContext::SetPkgContextInfo(std::map<std::string, std::string>& pkgContextInfoJsonStringMap,
    std::map<std::string, std::string>& packageNameList)
{
    const string path = CommandParser::GetInstance().GetAppResourcePath() +
        FileSystem::GetSeparator() + "module.json";
    string moduleJsonStr = JsonReader::ReadFile(path);
    if (moduleJsonStr.empty()) {
        ELOG("Get module.json content empty.");
    }
    Json2::Value rootJson1 = JsonReader::ParseJsonData2(moduleJsonStr);
    if (rootJson1.IsNull() || !rootJson1.IsValid() || !rootJson1.IsMember("module")) {
        ELOG("Get module.json content failed.");
        return;
    }
    if (!rootJson1["module"].IsMember("name") || !rootJson1["module"]["name"].IsString()) {
        return;
    }
    string moduleName = rootJson1["module"]["name"].AsString();
    if (rootJson1["module"].IsMember("packageName") && rootJson1["module"]["packageName"].IsString()) {
        string pkgName = rootJson1["module"]["packageName"].AsString();
        packageNameList = {{moduleName, pkgName}};
    }
    std::string jsonPath = CommandParser::GetInstance().GetLoaderJsonPath();
    std::string flag = "loader.json";
    int idx = jsonPath.find_last_of(flag);
    std::string dirPath = jsonPath.substr(0, idx - flag.size() + 1); // 1 is for \ or /
    std::string ctxPath = dirPath + "pkgContextInfo.json";
    string ctxInfoJsonStr = JsonReader::ReadFile(ctxPath);
    if (ctxInfoJsonStr.empty()) {
        ELOG("Get pkgContextInfo.json content empty.");
        return;
    }
    pkgContextInfoJsonStringMap = {{moduleName, ctxInfoJsonStr}};
}
}