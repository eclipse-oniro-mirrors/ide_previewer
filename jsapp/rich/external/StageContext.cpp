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

void StageContext::ParseJsonFile(const std::string& filePath)
{
    string jsonStr = JsonReader::ReadFile(filePath);
    Json::Value rootJson = JsonReader::ParseJsonData(jsonStr);
    if (!rootJson) {
        ELOG("Get module.json content failed.");
        return;
    }
    if (!rootJson.isMember("app") || !rootJson.isMember("module")) {
        ELOG("Don't find app or module node in module.json.");
        return;
    }
    ParseAppInfo(rootJson["app"]);
    ParseHapModuleInfo(rootJson["module"]);
}

void StageContext::ParseAppInfo(const Json::Value& root)
{
    if (!root) {
        ELOG("The information of stage model app info is null.");
        return;
    }
    appInfo.apiReleaseType = JsonReader::GetString(root, "apiReleaseType");
    appInfo.bundleName = JsonReader::GetString(root, "bundleName");
    appInfo.compileSdkType = JsonReader::GetString(root, "compileSdkType");
    appInfo.compileSdkVersion = JsonReader::GetString(root, "compileSdkVersion");
    appInfo.debug = JsonReader::GetBool(root, "debug", false);
    appInfo.icon = JsonReader::GetString(root, "icon");
    appInfo.iconId = JsonReader::GetUInt(root, "iconId", 0);
    appInfo.label = JsonReader::GetString(root, "label");
    appInfo.labelId = JsonReader::GetUInt(root, "labelId", 0);
    appInfo.minAPIVersion = JsonReader::GetUInt(root, "minAPIVersion", 0);
    appInfo.targetAPIVersion = JsonReader::GetUInt(root, "targetAPIVersion", 0);
    appInfo.vendor = JsonReader::GetString(root, "vendor");
    appInfo.versionCode = JsonReader::GetUInt(root, "versionCode", 0);
    appInfo.versionName = JsonReader::GetString(root, "versionName");
    // from arkui
    appInfo.bundleType = JsonReader::GetString(root, "bundleType");
    appInfo.distributedNotificationEnabled = JsonReader::GetBool(root, "distributedNotificationEnabled", true);
}

void StageContext::ParseHapModuleInfo(const Json::Value& root)
{
    if (!root) {
        ELOG("The information of stage model hap module info is null.");
        return;
    }
    ParseAbilityInfo(root);
    hapModuleInfo.compileMode = JsonReader::GetString(root, "compileMode");
    hapModuleInfo.deliveryWithInstall = JsonReader::GetBool(root, "deliveryWithInstall", false);
    hapModuleInfo.description = JsonReader::GetString(root, "description");
    hapModuleInfo.descriptionId = JsonReader::GetUInt(root, "descriptionId", 0);
    std::vector<std::string> deviceTypes;
    std::unique_ptr<Json::Value> deviceTypesArr = JsonReader::GetArray(root, "deviceTypes");
    if (deviceTypesArr) {
        for (auto index = 0; index < JsonReader::GetArraySize(*deviceTypesArr); ++index) {
            deviceTypes.push_back((*deviceTypesArr)[index].asString());
        }
    }
    hapModuleInfo.deviceTypes = deviceTypes;
    hapModuleInfo.installationFree = JsonReader::GetBool(root, "installationFree", false);
    hapModuleInfo.mainElement = JsonReader::GetString(root, "mainElement");
    hapModuleInfo.name = JsonReader::GetString(root, "name");
    hapModuleInfo.pages = JsonReader::GetString(root, "pages");
    hapModuleInfo.type = JsonReader::GetString(root, "type");
    hapModuleInfo.virtualMachine = JsonReader::GetString(root, "virtualMachine");
    hapModuleInfo.srcEntry = JsonReader::GetString(root, "srcEntry");
    // from arkui
    std::unique_ptr<Json::Value> metaData = JsonReader::GetArray(root, "metadata");
    if (metaData) {
        for (auto index = 0; index < JsonReader::GetArraySize(*metaData); ++index) {
            if ((*metaData)[index] && JsonReader::GetString((*metaData)[index], "name") == "ArkTSPartialUpdate") {
                hapModuleInfo.isPartialUpdate = (JsonReader::GetString((*metaData)[index], "value", "true") != "false");
            }
        }
    }
}

void StageContext::ParseAbilityInfo(const Json::Value& root)
{
    std::unique_ptr<Json::Value> abilitiesArr = JsonReader::GetArray(root, "abilities");
    std::vector<AbilityInfo> abilities;
    if (!abilitiesArr) {
        return;
    }
    for (auto index = 0; index < JsonReader::GetArraySize(*abilitiesArr); ++index) {
        AbilityInfo ability;
        ability.description = JsonReader::GetString((*abilitiesArr)[index], "description");
        ability.descriptionId = JsonReader::GetUInt((*abilitiesArr)[index], "descriptionId", 0);
        ability.exported = JsonReader::GetBool((*abilitiesArr)[index], "exported");
        ability.icon = JsonReader::GetString((*abilitiesArr)[index], "icon");
        ability.iconId = JsonReader::GetUInt((*abilitiesArr)[index], "iconId");
        ability.label = JsonReader::GetString((*abilitiesArr)[index], "label");
        ability.labelId = JsonReader::GetUInt((*abilitiesArr)[index], "labelId");
        ability.name = JsonReader::GetString((*abilitiesArr)[index], "name");
        std::unique_ptr<Json::Value> skillsArr = JsonReader::GetArray((*abilitiesArr)[index], "skills");
        std::vector<SkillInfo> skills;
        ParseSkillsInfo(skillsArr, skills);
        ability.skills = skills;
        ability.srcEntrty = JsonReader::GetString((*abilitiesArr)[index], "srcEntrty");
        ability.startWindowBackground = JsonReader::GetString((*abilitiesArr)[index], "startWindowBackground");
        ability.startWindowBackgroundId = JsonReader::GetUInt((*abilitiesArr)[index], "startWindowBackgroundId");
        ability.startWindowIcon = JsonReader::GetString((*abilitiesArr)[index], "startWindowIcon");
        ability.startWindowIconId = JsonReader::GetUInt((*abilitiesArr)[index], "startWindowIconId");
        abilities.push_back(ability);
    }
    hapModuleInfo.abilities = abilities;
}

void StageContext::ParseSkillsInfo(const std::unique_ptr<Json::Value>& skillsArr,
    std::vector<SkillInfo>& skills)
{
    if (!skillsArr) {
        return;
    }
    for (auto index = 0; index < JsonReader::GetArraySize(*skillsArr); ++index) {
        SkillInfo skill;
        std::unique_ptr<Json::Value> actionsArr = JsonReader::GetArray((*skillsArr)[index], "actions");
        if (actionsArr) {
            std::vector<std::string> actions;
            for (auto index = 0; index < JsonReader::GetArraySize(*actionsArr); ++index) {
                actions.push_back((*actionsArr)[index].asString());
            }
            skill.actions = actions;
        }
        std::unique_ptr<Json::Value> entitiesArr = JsonReader::GetArray((*skillsArr)[index], "entities");
        if (entitiesArr) {
            std::vector<std::string> entities;
            for (auto index = 0; index < JsonReader::GetArraySize(*entitiesArr); ++index) {
                entities.push_back((*entitiesArr)[index].asString());
            }
            skill.entities = entities;
        }
        skills.push_back(skill);
    }
}

const AppInfo& StageContext::GetAppInfo() const
{
    return appInfo;
}

const HapModuleInfo& StageContext::GetHapModuleInfo() const
{
    return hapModuleInfo;
}

const AbilityInfo& StageContext::GetAbilityInfo(const std::string srcEntryVal) const
{
    for (AbilityInfo ability : hapModuleInfo.abilities) {
        if (srcEntryVal == ability.srcEntrty) {
            return ability;
        }
    }
    return hapModuleInfo.abilities[0];
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
    SetMiddlePath(assetPath);
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
    std::string abcPath = modulePath + middlePath + "/modules.abc";
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

bool StageContext::ContainsRelativePath(const std::string& path) const
{
    return (path.find("../") != std::string::npos || path.find("./") != std::string::npos);
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

void StageContext::SetMiddlePath(const std::string& assetPath)
{
    std::string::size_type pos = assetPath.find_last_of(FileSystem::GetSeparator().c_str());
    std::string::size_type count = 0;
    int upwardLevel = 5;
    while (count < upwardLevel) {
        if (pos == std::string::npos) {
            ELOG("set middlePath:%s failed.");
            return;
        }
        pos = assetPath.find_last_of(FileSystem::GetSeparator().c_str(), pos - 1);
        ++count;
    }
    middlePath = assetPath.substr(pos);
    ILOG("set middlePath:%s successed.", middlePath.c_str());
}
}