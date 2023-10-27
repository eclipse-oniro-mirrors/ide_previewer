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

#ifndef STAGE_CONTEXT_H
#define STAGE_CONTEXT_H

#include <string>
#include <vector>
#include <optional>
#include <map>
#include <memory>

namespace Json {
    class Value;
}

namespace OHOS::Ide {
struct SkillInfo {
    std::vector<std::string> actions;
    std::vector<std::string> entities;
};

struct AbilityInfo {
    std::string description;
    uint32_t descriptionId;
    bool exported = false;
    std::string icon;
    uint32_t iconId = 0;
    std::string label;
    uint32_t labelId = 0;
    std::string name;
    std::vector<SkillInfo> skills;
    std::string srcEntrty;
    std::string startWindowBackground;
    uint32_t startWindowBackgroundId;
    std::string startWindowIcon;
    uint32_t startWindowIconId;
};

struct HapModuleInfo {
    std::vector<AbilityInfo> abilities;
    std::string compileMode;
    bool deliveryWithInstall = false;
    std::vector<std::string> dependencies;
    std::string description;
    uint32_t descriptionId;
    std::vector<std::string> deviceTypes;
    bool installationFree = false;
    std::string mainElement;
    std::string name;
    std::string pages;
    std::string type;
    std::string virtualMachine;
    std::string srcEntry;
    // from arkui
    bool isPartialUpdate = true;
    uint32_t labelId = 0;
};

struct AppInfo {
    // form mudule.json
    std::string apiReleaseType;
    std::string bundleName;
    std::string compileSdkType;
    std::string compileSdkVersion;
    bool debug = false;
    std::string icon;
    uint32_t iconId = 0;
    std::string label;
    uint32_t labelId = 0;
    uint32_t minAPIVersion = 0;
    uint32_t targetAPIVersion = 0;
    std::string vendor;
    uint32_t versionCode = 0;
    std::string versionName;
    // from arkui
    std::string bundleType;
    bool distributedNotificationEnabled = true;
};

class StageContext {
public:
    static StageContext& GetInstance();
    void ParseJsonFile(const std::string& filePath);
    void ParseAppInfo(const Json::Value& root);
    void ParseHapModuleInfo(const Json::Value& root);
    void ParseAbilityInfo(const Json::Value& root);
    void ParseSkillsInfo(const std::unique_ptr<Json::Value>& skillsArr, std::vector<SkillInfo>& skills);
    const AppInfo& GetAppInfo() const;
    const HapModuleInfo& GetHapModuleInfo() const;
    const AbilityInfo& GetAbilityInfo(const std::string srcEntryVal) const;
    // for Previewer
    void SetLoaderJsonPath(const std::string& assetPath);
    void GetModulePathMapFromLoaderJson();
    void ReleaseHspBuffers();
    // for ArkUI and Ability
    std::vector<uint8_t>* GetModuleBuffer(const std::string& inputPath);
    const std::optional<std::vector<uint8_t>> ReadFileContents(const std::string& filePath) const;
private:
    StageContext() = default;
    ~StageContext() = default;
    AppInfo appInfo;
    HapModuleInfo hapModuleInfo;
    bool ContainsRelativePath(const std::string& path) const;
    std::map<std::string, std::string> GetModulePathMap() const;
    std::string loaderJsonPath;
    std::map<std::string, std::string> modulePathMap;
    std::vector<std::vector<uint8_t>*> hspBufferPtrsVec;
    void SetMiddlePath(const std::string& assetPath);
    std::string middlePath;
};
}
#endif // STAGE_CONTEXT_H