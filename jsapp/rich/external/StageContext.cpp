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
#include <fstream>
#include "json.h"
#include "JsonReader.h"
#include "FileSystem.h"
#include "TraceTool.h"
#include "PreviewerEngineLog.h"
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
    size_t pos = assetPath.rfind(flag);
    if (pos == std::string::npos) {
        ELOG("assetPath: %s format error.", assetPath.c_str());
        return;
    }
    std::string assetDir = assetPath.substr(0, pos);
    loaderJsonPath = assetDir + "loader/default/loader.json";
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
    std::string moduleName = inputPath.substr(pos + spliter.size());
    ILOG("moduleName is:%s.", moduleName.c_str());
    if (modulePathMap.empty()) {
        ELOG("modulePathMap is empty.");
    }
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
    std::string abcPath = modulePath + "/.preview/default/intermediates/assets/default/ets/modules.abc";
    if (!FileSystem::IsFileExists(abcPath)) {
        ELOG("the abcPath is not exist.");
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

bool StageContext::ContainsRelativePath(const std::string& path) const
{
    return (path.find("../") != std::string::npos || path.find("./") != std::string::npos);
}
}