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

#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <map>
#include <string>
#include <vector>

class CommandInfo {
public:
    std::string deviceType;
    std::string pages;
    std::string appResourcePath;
    bool isCardDisplay;
    std::string containerSdkPath;
    bool isComponentMode;
    std::string loaderJsonPath;
    std::string abilityPath;
    std::string abilityName;
    std::string configPath;
    std::string screenShape;
    int32_t orignalResolutionWidth;
    int32_t orignalResolutionHeight;
    int32_t compressionResolutionWidth;
    int32_t compressionResolutionHeight;
};

class FoldInfo {
public:
    bool foldable;
    std::string foldStatus;
    int32_t foldResolutionWidth;
    int32_t foldResolutionHeight;
};

class CommandParser {
public:
    CommandParser(const CommandParser&) = delete;
    CommandParser& operator=(const CommandParser&) = delete;
    static CommandParser& GetInstance();
    bool ProcessCommand(std::vector<std::string> strs);
    bool IsCommandValid();
    bool IsSet(std::string key);
    std::string Value(std::string key);
    std::vector<std::string> Values(std::string key);
    void Register(std::string key, uint32_t argc, std::string help);
    bool IsResolutionValid(int32_t resolution) const;
    int32_t GetOrignalResolutionWidth() const;
    int32_t GetOrignalResolutionHeight() const;
    int32_t GetCompressionResolutionWidth() const;
    int32_t GetCompressionResolutionHeight() const;
    uint32_t GetJsHeapSize() const;
    std::string GetAppName() const;
    bool IsSendJSHeap() const;
    std::string GetDeviceType() const;
    bool IsRegionRefresh() const;
    bool IsCardDisplay() const;
    std::string GetConfigPath() const;
    std::string GetProjectID() const;
    enum class ScreenMode { DYNAMIC = 0, STATIC = 1};
    CommandParser::ScreenMode GetScreenMode() const;
    std::string GetConfigChanges() const;
    std::string GetAppResourcePath() const;
    std::string GetScreenShape() const;
    std::string GetProjectModel() const;
    int GetProjectModelEnumValue() const;
    std::string GetProjectModelEnumName(int enumValue) const;
    std::string GetPages() const;
    std::string GetContainerSdkPath() const;
    bool CheckParamInvalidity(std::string param, bool isNum);
    bool IsComponentMode() const;
    std::string GetAbilityPath() const;
    std::string GetAbilityName() const;
    bool IsStaticCard() const;
    bool IsMainArgLengthInvalid(const char* str) const;
    bool IsFoldable() const;
    std::string GetFoldStatus() const;
    int32_t GetFoldResolutionWidth() const;
    int32_t GetFoldResolutionHeight() const;
    std::string GetLoaderJsonPath() const;
    int ParseArgs(int argc, char* argv[]);
    void GetCommandInfo(CommandInfo& info) const;
    void GetFoldInfo(FoldInfo& info) const;
#ifdef COMPONENT_TEST_ENABLED
    std::string GetComponentTestConfig() const;
#endif // COMPONENT_TEST_ENABLED

private:
    CommandParser();
    ~CommandParser() {}
    std::string errorInfo;
    std::map<std::string, std::vector<std::string>> argsMap;
    std::map<std::string, uint32_t> regsArgsCountMap;
    std::map<std::string, std::string> regsHelpMap;
    static CommandParser* example;
    const std::vector<std::string> supportedDevices = {
        "liteWearable",
        "smartVision",
        "wearable",
        "tv",
        "phone",
        "tablet",
        "car",
        "2in1",
        "default"
    };
    const std::vector<std::string> cardDisplayDevices = {"phone", "tablet", "wearable", "car", "tv", "2in1", "default"};
    const std::vector<std::string> projectModels = {"FA", "Stage"};
    const int MIN_PORT = 1024;
    const int MAX_PORT = 65535;
    const int32_t MIN_RESOLUTION = 1;
    const int32_t MAX_RESOLUTION = 3840;
    const int MAX_JSHEAPSIZE = 512 * 1024;
    const int MIN_JSHEAPSIZE = 48 * 1024;
    const size_t MAX_NAME_LENGTH = 256;
    bool isSendJSHeap;
    int32_t orignalResolutionWidth;
    int32_t orignalResolutionHeight;
    int32_t compressionResolutionWidth;
    int32_t compressionResolutionHeight;
    uint32_t jsHeapSize;
    std::string deviceType;
    std::string screenShape;
    std::string appName;
    std::string urlPath;
    std::string configPath;
    bool isRegionRefresh;
    bool isCardDisplay;
    std::string projectID;
    CommandParser::ScreenMode screenMode;
    std::string configChanges;
    std::string appResourcePath;
    std::string projectModel;
    std::string pages;
    std::string containerSdkPath;
    std::string regex4Num = "^(0|[1-9][0-9]*)(\\.[0-9]+)?$";
    std::string regex4Str = "^(?:[a-zA-Z0-9-_./\\s]+)$";
    bool isComponentMode;
    std::string abilityPath;
    std::string abilityName;
#ifdef COMPONENT_TEST_ENABLED
    std::string componentTestConfig;
#endif // COMPONENT_TEST_ENABLED
    bool staticCard;
    const size_t maxMainArgLength = 1024;
    bool foldable;
    std::string foldStatus;
    int32_t foldResolutionWidth;
    int32_t foldResolutionHeight;
    std::string loaderJsonPath;

    bool IsDebugPortValid();
    bool IsAppPathValid();
    bool IsAppNameValid();
    bool IsResolutionValid();
    bool IsConfigPathValid();
    bool IsAppResourcePathValid();
    bool IsResolutionRangeValid(std::string value);
    bool IsResolutionArgValid(std::string command);
    bool IsJsHeapValid();
    bool IsJsHeapFlagValid();
    bool IsScreenShapeValid();
    bool IsDeviceValid();
    bool IsUrlValid();
    bool IsRefreshValid();
    bool IsCardValid();
    bool IsProjectIDValid();
    bool IsColorModeValid();
    bool IsAceVersionValid();
    bool IsOrientationValid();
    bool IsWebSocketPortValid();
    bool IsScreenModeValid();
    bool IsProjectModelValid();
    bool IsPagesValid();
    bool IsLanguageValid();
    bool IsTracePipeNameValid();
    bool IsLocalSocketNameValid();
    bool IsScreenDensityValid();
    bool IsConfigChangesValid();
    bool IsContainerSdkPathValid();
    bool IsComponentModeValid();
    bool IsAbilityPathValid();
    bool IsAbilityNameValid();
    bool IsStaticCardValid();
    bool IsFoldableValid();
    bool IsFoldStatusValid();
    bool IsFoldResolutionValid();
    bool IsLoaderJsonPathValid();
    std::string HelpText();
    void ProcessingCommand(const std::vector<std::string>& strs);
};

#endif // COMMANDPARSER_H
