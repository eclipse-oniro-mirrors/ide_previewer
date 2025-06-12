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
#include <algorithm>
#include "JsonReader.h"
#include "FileSystem.h"
#include "TraceTool.h"
#include "PreviewerEngineLog.h"
#include "CommandParser.h"
#include "zlib.h"
#include "contrib/minizip/unzip.h"

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
    std::string jsonStr = JsonReader::ReadFile(loaderJsonPath);
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
    Json2::Value jsonObjOhm = rootJson["harNameOhmMap"];
    if (rootJson.IsMember("hspNameOhmMap")) {
        if (!rootJson["hspNameOhmMap"].IsNull() && rootJson["hspNameOhmMap"].IsValid()) {
            ILOG("hspNameOhmMap is valid");
            jsonObjOhm = rootJson["hspNameOhmMap"];
        }
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
    std::string jsonStr = JsonReader::ReadFile(hspConfigPath);
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
        if (ptr) {
            delete ptr;
        }
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
            if (GetLocalModuleBuffer(moduleName) == nullptr) {
                return GetCloudModuleBuffer(moduleName);
            } else {
                return GetLocalModuleBuffer(moduleName);
            }
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
    std::vector<uint8_t> *buf = new(std::nothrow) std::vector<uint8_t>(opt.value());
    if (!buf) {
        ELOG("Memory allocation failed: buf.");
    }
    hspBufferPtrsVec.push_back(buf);
    return buf;
}

std::string StageContext::GetCloudHspVersion(const std::string& hspPath, const std::string& actualName)
{
    std::string flag = "@";
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
    std::string moduleHspPath = GetCloudModuleHspPath(actualName);
    std::string projectHspPath = GetCloudProjectHspPath(actualName);
    ILOG("moduleHspPath:%s, projectHspPath:%s", moduleHspPath.c_str(), projectHspPath.c_str());
    if (moduleHspPath.empty() || !FileSystem::IsDirectoryExists(moduleHspPath)) {
        return projectHspPath; // 模块级不存在，加载项目级
    }
    if (projectHspPath.empty() || !FileSystem::IsDirectoryExists(projectHspPath)) {
        return moduleHspPath; // 模块级存在，项目级不存在，加载模块级
    }
    // 模块级和项目级都存在，加载版本号高的
    std::string moduleHspVersion = GetCloudHspVersion(moduleHspPath, actualName);
    std::string projectHspVersion = GetCloudHspVersion(projectHspPath, actualName);
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
    std::string flag = "@";
    std::string partName = moduleName + flag;
    return FileSystem::FindSubfolderByName(hspDir, partName);
}

std::vector<uint8_t>* StageContext::GetModuleBufferFromHsp(const std::string& hspFilePath,
    const std::string& fileName)
{
    unzFile zipfile = unzOpen2(hspFilePath.c_str(), nullptr);
    if (zipfile == NULL) {
        ELOG("Failed to open the zip file: %s\n", hspFilePath.c_str());
        return nullptr;
    }

    if (unzLocateFile(zipfile, fileName.c_str(), 1) != UNZ_OK) {
        ELOG("Failed to locate the file: %s\n", fileName.c_str());
        unzClose(zipfile);
        return nullptr;
    }

    unz_file_info file_info;
    if (unzGetCurrentFileInfo(zipfile, &file_info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) {
        ELOG("Failed to get the file info: %s\n", fileName.c_str());
        unzClose(zipfile);
        return nullptr;
    }

    if (unzOpenCurrentFile(zipfile) != UNZ_OK) {
        ELOG("Failed to open the file: %s\n", fileName.c_str());
        unzClose(zipfile);
        return nullptr;
    }

    char buffer[1024];
    int bytesRead;
    std::vector<uint8_t>* fileContent = new(std::nothrow) std::vector<uint8_t>();
    if (!fileContent) {
        ELOG("Memory allocation failed: fileContent.");
        return fileContent;
    }
    while ((bytesRead = unzReadCurrentFile(zipfile, buffer, sizeof(buffer))) > 0) {
        fileContent->insert(fileContent->end(), buffer, buffer + bytesRead);
    }
    hspBufferPtrsVec.push_back(fileContent);
    unzCloseCurrentFile(zipfile);
    unzClose(zipfile);

    ILOG("File extracted and content saved: %s\n", fileName.c_str());
    return fileContent;
}

bool StageContext::ContainsRelativePath(const std::string& path) const
{
    std::string flg1 = ".." + FileSystem::GetSeparator();
    std::string flg2 = "." + FileSystem::GetSeparator();
    return (path.find(flg1) != std::string::npos || path.find(flg2) != std::string::npos);
}

std::map<std::string, std::string> StageContext::ParseMockJsonFile(const std::string& mockJsonFilePath)
{
    std::map<std::string, std::string> mapInfo;
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
    std::string flag = "";
    std::string bundleFlag = "/" + input + "/";
    std::string normalizedFlag = "N&" + input + "&";
    for (const auto& pair : hspNameOhmMap) {
        if (pair.second.find("@normalized") == 0) {
            flag = normalizedFlag;
        } else {
            flag = bundleFlag;
        }
        if (pair.second.find(flag) != std::string::npos) {
            std::string actualName = pair.first;
            if (actualName.find('/') != std::string::npos) { // 以组织名开头的包
                std::replace(actualName.begin(), actualName.end(), '/', '+');
            }
            if (num == 0) {
                ret = actualName;
            }
            num++;
            WLOG("find hsp actual name:%s", actualName.c_str());
        }
    }
    return num;
}

std::vector<uint8_t>* StageContext::GetSystemModuleBuffer(const std::string& inputPath,
    const std::string& moduleName)
{
    std::string head = "com.huawei";
    std::string tail = moduleName;
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
    const std::string path = CommandParser::GetInstance().GetAppResourcePath() +
        FileSystem::GetSeparator() + "module.json";
    std::string moduleJsonStr = JsonReader::ReadFile(path);
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
    std::string moduleName = rootJson1["module"]["name"].AsString();
    if (rootJson1["module"].IsMember("packageName") && rootJson1["module"]["packageName"].IsString()) {
        std::string pkgName = rootJson1["module"]["packageName"].AsString();
        packageNameList = {{moduleName, pkgName}};
    }
    std::string jsonPath = CommandParser::GetInstance().GetLoaderJsonPath();
    std::string flag = "loader.json";
    int idx = jsonPath.find_last_of(flag);
    std::string dirPath = jsonPath.substr(0, idx - flag.size() + 1); // 1 is for \ or /
    std::string ctxPath = dirPath + "pkgContextInfo.json";
    std::string ctxInfoJsonStr = JsonReader::ReadFile(ctxPath);
    if (ctxInfoJsonStr.empty()) {
        ELOG("Get pkgContextInfo.json content empty.");
        return;
    }
    pkgContextInfoJsonStringMap = {{moduleName, ctxInfoJsonStr}};
    Json2::Value rootJson = JsonReader::ParseJsonData2(ctxInfoJsonStr);
    if (rootJson.IsNull() || !rootJson.IsValid()) {
        ELOG("parse ctx info json failed.");
        return;
    }
    for (const auto& element : rootJson.GetMemberNames()) {
        if (!rootJson[element]["moduleName"].IsString()) {
            return;
        }
        packageNameMap[element] = rootJson[element]["moduleName"].AsString();
    }
}

bool StageContext::UnzipHspFile(const std::string& hspFilePath, const std::string& writePath,
    const std::vector<std::string> fileNames)
{
    unzFile zipfile = unzOpen(hspFilePath.c_str());
    if (zipfile == NULL) {
        ELOG("Error: Unable to open zip file %s\n", hspFilePath.c_str());
        return false;
    }

    bool isUnzipSuccess = true;
    for (const auto& fileName : fileNames) {
        if (unzLocateFile(zipfile, fileName.c_str(), 1) != UNZ_OK) {
            ELOG("Failed to locate the file: %s\n", fileName.c_str());
            unzClose(zipfile);
            return false;
        }
        unz_file_info file_info;
        if (unzGetCurrentFileInfo(zipfile, &file_info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) {
            ELOG("Failed to get the file info: %s\n", fileName.c_str());
            unzClose(zipfile);
            return false;
        }
        if (unzOpenCurrentFile(zipfile) != UNZ_OK) {
            ELOG("Failed to open the file: %s\n", fileName.c_str());
            unzClose(zipfile);
            return false;
        }

        std::string filePath = writePath + FileSystem::GetSeparator() + fileName;
        FILE *outputFile = fopen(filePath.c_str(), "wb");
        if (outputFile == NULL) {
            ELOG("Error: Unable to open output file %s\n", fileName.c_str());
            unzCloseCurrentFile(zipfile);
            break;
        }

        char buffer[4096];
        int bytesRead = 0;
        do {
            bytesRead = unzReadCurrentFile(zipfile, buffer, sizeof(buffer));
            if (bytesRead < 0 || bytesRead != fwrite(buffer, 1, bytesRead, outputFile)) {
                ELOG("Failed to unzip all elements to the output file.\n");
                isUnzipSuccess = false;
                break;
            }
        } while (bytesRead > 0);

        fclose(outputFile);
        unzCloseCurrentFile(zipfile);
        if (!isUnzipSuccess) {
            break;
        }
    }

    unzClose(zipfile);
    return isUnzipSuccess;
}

void StageContext::GetModuleInfo(std::vector<HspInfo>& dependencyHspInfos)
{
    if (hspNameOhmMap.empty()) {
        ELOG("hspNameOhmMap is empty.");
        return;
    }
    for (const auto &pair : hspNameOhmMap) {
        std::string packageName = pair.first;
        HspInfo hspInfo;
        if (packageNameMap.find(packageName) != packageNameMap.end()) {
            hspInfo.moduleName = packageNameMap[packageName];
            GetHspinfo(packageName, hspInfo);
        }
        dependencyHspInfos.push_back(hspInfo);
    }
}

void StageContext::GetHspinfo(const std::string& packageName, HspInfo& hspInfo)
{
    if (modulePathMap.count(hspInfo.moduleName) > 0) {
        bool ret = GetLocalModuleInfo(hspInfo);
        if (!ret) {
            GetCloudModuleInfo(packageName, hspInfo);
        }
    } else {
        GetCloudModuleInfo(packageName, hspInfo);
    }
}

bool StageContext::GetLocalModuleInfo(HspInfo& hspInfo)
{
    std::string modulePath = StageContext::GetInstance().modulePathMap[hspInfo.moduleName];
    if (modulePath.empty()) {
        ELOG("modulePath is empty.");
        return false;
    }
    ILOG("get modulePath: %s successfully.", modulePath.c_str());
    if (!FileSystem::IsDirectoryExists(modulePath)) {
        ELOG("don't find moduleName: %s in modulePathMap from loader.json.", hspInfo.moduleName.c_str());
        return false;
    }
    if (ContainsRelativePath(modulePath)) {
        ELOG("modulePath format error: %s.", modulePath.c_str());
        return false;
    }
    std::string separator = FileSystem::GetSeparator();
    // 读取hsp的module.json和resources.index
    std::string hspConfigPath = modulePath + separator + ".preview" + separator + "default" +
        separator + "intermediates" + separator + "res" + separator + "default";
    std::string moduleJsonPath = hspConfigPath + separator + "module.json";
    std::string resources = hspConfigPath + separator + "resources.index";
    if (!FileSystem::IsFileExists(resources)) {
        ELOG("The resources.index file is not exist.");
        return false;
    }
    hspInfo.resourcePath = resources;
    if (!FileSystem::IsFileExists(moduleJsonPath)) {
        ELOG("The module.json file is not exist.");
        return false;
    }
    std::optional<std::vector<uint8_t>> ctx = OHOS::Ide::StageContext::GetInstance().ReadFileContents(moduleJsonPath);
    if (ctx.has_value()) {
        hspInfo.moduleJsonBuffer = ctx.value();
    } else {
        ELOG("get %s module.json content failed", hspInfo.moduleName.c_str());
    }
    return true;
}

bool StageContext::GetCloudModuleInfo(const std::string& packageName, HspInfo& hspInfo)
{
    std::string hspPath = GetActualCloudHspDir(packageName);
    ILOG("get hspPath:%s actualName:%s", hspPath.c_str(), packageName.c_str());
    if (!FileSystem::IsDirectoryExists(hspPath)) {
        ELOG("hspPath: %s is not exist.", hspPath.c_str());
        return false;
    }
    std::string moduleHspFile = hspPath + "/" + packageName + ".hsp";
    ILOG("get moduleHspFile:%s.", moduleHspFile.c_str());
    if (!FileSystem::IsFileExists(moduleHspFile)) {
        ELOG("the moduleHspFile:%s is not exist.", moduleHspFile.c_str());
        return false;
    }
    std::string separator = FileSystem::GetSeparator();
    // 读取hsp的module.json和resources.index
    std::vector<std::string> fileNames = {"module.json", "resources.index"};
    if (!UnzipHspFile(moduleHspFile, hspPath, fileNames)) {
        ELOG("unzip hsp file failed.");
        return false;
    };
    std::string modulePath = hspPath + separator + "module.json";
    std::string resources = hspPath + separator + "resources.index";
    if (!FileSystem::IsFileExists(resources)) {
        ELOG("The resources.index file is not exist.");
        return false;
    }
    hspInfo.resourcePath = resources;
    if (!FileSystem::IsFileExists(modulePath)) {
        ELOG("The module.json file is not exist.");
        return false;
    }
    std::optional<std::vector<uint8_t>> ctx = OHOS::Ide::StageContext::GetInstance().ReadFileContents(modulePath);
    if (ctx.has_value()) {
        hspInfo.moduleJsonBuffer = ctx.value();
    } else {
        ELOG("get %s module.json content failed", hspInfo.moduleName.c_str());
    }
    return true;
}
}