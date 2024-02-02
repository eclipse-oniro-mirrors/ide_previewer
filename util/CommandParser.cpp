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

#include "CommandParser.h"

#include <algorithm>
#include <cstdlib>
#include <regex>
#include "FileSystem.h"
#include "PreviewerEngineLog.h"
#include "TraceTool.h"

using namespace std;
CommandParser* CommandParser::example = nullptr;
CommandParser::CommandParser()
    : isSendJSHeap(true),
      orignalResolutionWidth(0),
      orignalResolutionHeight(0),
      compressionResolutionWidth(0),
      compressionResolutionHeight(0),
      jsHeapSize(MAX_JSHEAPSIZE),
      deviceType("liteWearable"),
      screenShape("circle"),
      appName("undefined"),
      configPath(""),
      isRegionRefresh(false),
      isCardDisplay(false),
      projectID(""),
      screenMode(CommandParser::ScreenMode::DYNAMIC),
      configChanges(""),
      appResourcePath(""),
      projectModel("FA"),
      pages("main_pages"),
      containerSdkPath(""),
      isComponentMode(false),
      abilityPath(""),
      staticCard(false)
{
    Register("-j", 1, "Launch the js app in <directory>.");
    Register("-n", 1, "Set the js app name show on <window title>.");
    Register("-d", 0, "Run in debug mode and start debug server.");
    Register("-p", 1, "Config debug server to listen <port>.");
    Register("-s", 1, "Local socket name <socket-name> for command line interface.");
    Register("-v", 0, "Print the periviewer engine version.");
    Register("-h", 0, "Print the usage help.");
    Register("-or", 2, "Original resolution <width> <height>"); // 2 arguments
    Register("-cr", 2, "Compression resolution <width> <height>"); // 2 arguments
    Register("-f", 1, "config path <path>");
    Register("-hs", 1, "JS Heap <size>");
    Register("-hf", 1, "JS Send Heap <flag>");
    Register("-shape", 1, "Screen shape <shape>");
    Register("-device", 1, "Device type <type>");
    Register("-url", 1, "temp url");
    Register("-refresh", 1, "Screen <refresh mode>, support region and full");
    Register("-card", 1, "Controls the display <type> to switch between the app and card.");
    Register("-projectID", 1, "the ID of current project.");
    Register("-ts", 1, "Trace socket name");
    Register("-cm", 1, "Set colormode for the theme.");
    Register("-o", 1, "Set orientation for the display.");
    Register("-lws", 1, "Listening port of WebSocket");
    Register("-av", 1, "Set ace version.");
    Register("-l", 1, "Set language for startParam.");
    Register("-sd", 1, "Set screenDensity for Previewer.");
    Register("-sm", 1, "Set Screen picture transport mode, support dynamic and static");
    Register("-cc", 1, "Set Resource configChanges.");
    Register("-arp", 1, "Set App ResourcePath.");
    Register("-fs", 1, "Select Fonts sources.");
    Register("-pm", 1, "Set project model type.");
    Register("-pages", 1, "Set project's router config file path.");
    Register("-hsp", 1, "Set container sdk path.");
    Register("-cpm", 1, "Set previewer start mode.");
    Register("-abp", 1, "Set abilityPath for debug.");
    Register("-staticCard", 1, "Set card mode.");
    Register("-foldable", 1, "Set foldable for Previewer.");
    Register("-foldStatus", 1, "Set fold status for Previewer.");
    Register("-fr", 2, "Fold resolution <width> <height>"); // 2 arguments
    Register("-ljPath", 1, "Set loader.json path for Previewer");
}

CommandParser& CommandParser::GetInstance()
{
    static CommandParser instance;
    return instance;
}

/*
 * Parse user input and check parameter validity
 */
bool CommandParser::ProcessCommand(std::vector<std::string> strs)
{
    ProcessingCommand(strs);

    if (IsSet("v")) {
        ELOG("ProcessCommand Set -v!");
        return false;
    }

    if (IsSet("h")) {
        ELOG("ProcessCommand Set -h!");
        ELOG(HelpText().c_str());
        return false;
    }

    return true;
}

bool CommandParser::IsCommandValid()
{
    bool partRet = IsDebugPortValid() && IsAppPathValid() && IsAppNameValid() && IsResolutionValid();
    partRet = partRet && IsConfigPathValid() && IsJsHeapValid() && IsJsHeapFlagValid() && IsScreenShapeValid();
    partRet = partRet && IsDeviceValid() && IsUrlValid() && IsRefreshValid() && IsCardValid() && IsProjectIDValid();
    partRet = partRet && IsColorModeValid() && IsOrientationValid() && IsWebSocketPortValid() && IsAceVersionValid();
    partRet = partRet && IsScreenModeValid() && IsAppResourcePathValid() && IsLoaderJsonPathValid();
    partRet = partRet && IsProjectModelValid() && IsPagesValid() && IsContainerSdkPathValid();
    partRet = partRet && IsComponentModeValid() && IsAbilityPathValid() && IsStaticCardValid();
    partRet = partRet && IsFoldableValid() && IsFoldStatusValid() && IsFoldResolutionValid();
    if (partRet) {
        return true;
    }
    ELOG(errorInfo.c_str());
    ILOG(HelpText().c_str());
    TraceTool::GetInstance().HandleTrace("Invalid startup parameters");
    return false;
}

bool CommandParser::IsSet(string key)
{
    if (argsMap.find(string("-") + key) == argsMap.end()) {
        return false;
    }
    return true;
}

string CommandParser::Value(string key)
{
    auto args = argsMap[string("-") + key];
    if (args.size() > 0) {
        return args[0];
    }
    return string();
}

vector<string> CommandParser::Values(string key)
{
    if (argsMap.find(key) == argsMap.end()) {
        return vector<string>();
    }
    vector<string> args = argsMap[key];
    return args;
}

void CommandParser::Register(string key, uint32_t argc, string help)
{
    regsArgsCountMap[key] = argc;
    regsHelpMap[key] = help;
}

bool CommandParser::IsResolutionValid(int32_t resolution) const
{
    if (resolution >= MIN_RESOLUTION && resolution <= MAX_RESOLUTION) {
        return true;
    }
    return false;
}

string CommandParser::GetDeviceType() const
{
    return deviceType;
}

bool CommandParser::IsRegionRefresh() const
{
    return isRegionRefresh;
}

bool CommandParser::IsCardDisplay() const
{
    return isCardDisplay;
}

string CommandParser::GetConfigPath() const
{
    return configPath;
}

string CommandParser::GetProjectID() const
{
    return projectID;
}

string CommandParser::GetAppResourcePath() const
{
    return appResourcePath;
}

string CommandParser::GetScreenShape() const
{
    return screenShape;
}

string CommandParser::GetProjectModel() const
{
    return projectModel;
}

string CommandParser::GetPages() const
{
    return pages;
}

string CommandParser::GetContainerSdkPath() const
{
    return containerSdkPath;
}

CommandParser::ScreenMode CommandParser::GetScreenMode() const
{
    return screenMode;
}

string CommandParser::GetConfigChanges() const
{
    return configChanges;
}

int32_t CommandParser::GetOrignalResolutionWidth() const
{
    return orignalResolutionWidth;
}

int32_t CommandParser::GetOrignalResolutionHeight() const
{
    return orignalResolutionHeight;
}

int32_t CommandParser::GetCompressionResolutionWidth() const
{
    return compressionResolutionWidth;
}

int32_t CommandParser::GetCompressionResolutionHeight() const
{
    return compressionResolutionHeight;
}

uint32_t CommandParser::GetJsHeapSize() const
{
    return jsHeapSize;
}

string CommandParser::GetAppName() const
{
    return appName;
}

bool CommandParser::IsSendJSHeap() const
{
    return isSendJSHeap;
}

bool CommandParser::IsComponentMode() const
{
    return isComponentMode;
}

string CommandParser::GetAbilityPath() const
{
    return abilityPath;
}

bool CommandParser::IsStaticCard() const
{
    return staticCard;
}

bool CommandParser::IsDebugPortValid()
{
    if (IsSet("p")) {
        if (CheckParamInvalidity(Value("p"), true)) {
            errorInfo = "Launch -p parameters is not match regex.";
            return false;
        }
        int port = atoi(Value("p").c_str());
        if (port < MIN_PORT || port > MAX_PORT) {
            errorInfo =
                string("Debug server port out of range: " + to_string(MIN_PORT) + "-" + to_string(MAX_PORT) + ".");
            ELOG("Launch -p parameters abnormal!");
            return false;
        }
    }
    ILOG("CommandParser debug port: %s", Value("p").c_str());
    return true;
}

bool CommandParser::IsAppPathValid()
{
    if (!IsSet("j")) {
        errorInfo = string("No app path specified.");
        ELOG("Launch -j parameters abnormal!");
        return false;
    }
    string path = Value("j");
    if (!FileSystem::IsDirectoryExists(path)) {
        errorInfo = string("Js app path not exist.");
        ELOG("Launch -j parameters abnormal!");
        return false;
    }

    return true;
}

bool CommandParser::IsAppNameValid()
{
    if (IsSet("n")) {
        if (CheckParamInvalidity(Value("n"), false)) {
            errorInfo = "Launch -n parameters is not match regex.";
            return false;
        }
        size_t size = Value("n").size();
        if (size > MAX_NAME_LENGTH) {
            errorInfo = string("Js app name it too long, max: " + to_string(MAX_NAME_LENGTH) + ".");
            return false;
        }
        appName = Value("n");
    }
    ILOG("CommandParser app name: %s", appName.c_str());
    return true;
}

bool CommandParser::IsResolutionValid()
{
    if (IsSet("or") && IsSet("cr")) {
        if (IsResolutionArgValid(string("-or")) && IsResolutionArgValid(string("-cr"))) {
            orignalResolutionWidth = atoi(Values("-or")[0].c_str());
            orignalResolutionHeight = atoi(Values("-or")[1].c_str());
            compressionResolutionWidth = atoi(Values("-cr")[0].c_str());
            compressionResolutionHeight = atoi(Values("-cr")[1].c_str());
            ILOG("CommandParser resolution: %d %d %d %d", orignalResolutionWidth, orignalResolutionHeight,
                 compressionResolutionWidth, compressionResolutionHeight);
            return true;
        }
        ELOG("Launch -cr/-or parameters abnormal!");
        return false;
    }
    ELOG("Launch -cr/-or parameters abnormal!");
    errorInfo = string("Origin resolution and compress resolution must be setted.");
    return false;
}

bool CommandParser::IsJsHeapValid()
{
    if (IsSet("hs")) {
        if (CheckParamInvalidity(Value("hs"), true)) {
            errorInfo = "Launch -hs parameters is not match regex.";
            return false;
        }
        int size = atoi(Value("hs").c_str());
        if (size < MIN_JSHEAPSIZE || size > MAX_JSHEAPSIZE) {
            errorInfo = string("JS heap size out of range: " + to_string(MIN_JSHEAPSIZE) + "-" +
                               to_string(MAX_JSHEAPSIZE) + ".");
            ELOG("Launch -hs parameters abnormal!");
            return false;
        }
        jsHeapSize = static_cast<uint32_t>(size);
    }
    ILOG("CommandParser js heap: %d", jsHeapSize);
    return true;
}

bool CommandParser::IsJsHeapFlagValid()
{
    if (IsSet("hf")) {
        string flag = Value("hf");
        if (flag != "true" && flag != "false") {
            errorInfo = string("JS heap flag suported: true or false");
            ELOG("Launch -hs parameters abnormal!");
            return false;
        }
        isSendJSHeap = (flag == "true");
    }
    ILOG("CommandParser is send JS heap: %d", isSendJSHeap);
    return true;
}

bool CommandParser::IsScreenShapeValid()
{
    if (IsSet("shape")) {
        string shape = Value("shape");
        if (shape != "rect" && shape != "circle") {
            errorInfo = string("Screen shape suported: rect or circle");
            ELOG("The current device does not support, please upgrade the SDK!");
            return false;
        }
        screenShape = shape;
    }
    ILOG("CommandParser screen shape: %s", screenShape.c_str());
    return true;
}

bool CommandParser::IsDeviceValid()
{
    if (IsSet("device")) {
        auto iter = find(supportedDevices.begin(), supportedDevices.end(), Value("device"));
        if (iter == supportedDevices.end()) {
            errorInfo += string("Device type unsupport, please upgrade the Previewer SDK!");
            ELOG("Device type unsupport!");
            return false;
        }
    }
    deviceType = Value("device");
    ILOG("CommandParser device: %s", deviceType.c_str());
    return true;
}

bool CommandParser::IsUrlValid()
{
    urlPath = Value("url");
    if (urlPath.empty()) {
        errorInfo = "Launch -url parameters is empty.";
        return false;
    }
    ILOG("CommandParser url: %s", urlPath.c_str());
    return true;
}

bool CommandParser::IsConfigPathValid()
{
    if (!IsSet("f")) {
        return true;
    }

    string path = Value("f");
    if (!FileSystem::IsFileExists(path)) {
        errorInfo = string("The configuration file path does not exist.");
        ELOG("Launch -f parameters abnormal!");
        return false;
    }
    configPath = path;
    return true;
}

bool CommandParser::IsAppResourcePathValid()
{
    if (!IsSet("arp")) {
        return true;
    }

    string path = Value("arp");
    if (!FileSystem::IsDirectoryExists(path)) {
        errorInfo = string("The configuration appResource path does not exist.");
        ELOG("Launch -arp parameters abnormal!");
        return false;
    }
    appResourcePath = path;
    return true;
}

bool CommandParser::IsProjectModelValid()
{
    if (!IsSet("pm")) {
        return true;
    }

    string projectModelStr = Value("pm");
    auto iter = find(projectModels.begin(), projectModels.end(), projectModelStr);
    if (iter == projectModels.end()) {
        errorInfo = string("The project model does not exist.");
        ELOG("Launch -pm parameters abnormal!");
        return false;
    }

    projectModel = projectModelStr;
    ILOG("CommandParser projectModel: %s", projectModelStr.c_str());
    return true;
}

bool CommandParser::IsPagesValid()
{
    if (!IsSet("pages")) {
        return true;
    }
    pages = Value("pages");
    if (CheckParamInvalidity(pages, false)) {
        errorInfo = "Launch -pages parameters is not match regex.";
        return false;
    }
    ILOG("CommandParser pages: %s", pages.c_str());
    return true;
}

bool CommandParser::IsResolutionArgValid(string command)
{
    vector<string> value = Values(command);
    uint32_t size = regsArgsCountMap[command];
    if (value.size() != size) {
        errorInfo = string("Invalid argument's count.");
        return false;
    }
    if (IsResolutionRangeValid(value[0]) && IsResolutionRangeValid(value[1])) {
        return true;
    }
    return false;
}

bool CommandParser::IsResolutionRangeValid(string value)
{
    if (CheckParamInvalidity(value, true)) {
        errorInfo = "Launch -or/-cr or -fr parameters is not match regex.";
        return false;
    }
    int32_t temp = atoi(value.c_str());
    if (!IsResolutionValid(temp)) {
        errorInfo = string("Resolution range " + to_string(MIN_RESOLUTION) + "-" + to_string(MAX_RESOLUTION) + ".");
        return false;
    }
    return true;
}

bool CommandParser::IsRefreshValid()
{
    if (!IsSet("refresh")) {
        return true;
    }

    string refresh = Value("refresh");
    if (refresh != "region" && refresh != "full") {
        errorInfo = string("The refresh argument unsupported.");
        ELOG("Launch -refresh parameters abnormal!");
        return false;
    }
    if (refresh == "region") {
        isRegionRefresh = true;
    }
    return true;
}

bool CommandParser::IsCardValid()
{
    if (!IsSet("card")) {
        return true;
    }

    string card = Value("card");
    if (card != "true" && card != "false") {
        errorInfo = string("The card argument unsupported.");
        ELOG("Launch -card parameters abnormal!");
        return false;
    }

    std::string devicetype = GetDeviceType();
    auto iter = find(cardDisplayDevices.begin(), cardDisplayDevices.end(), devicetype);
    if (iter != cardDisplayDevices.end() && card == "true") {
        isCardDisplay = true;
    }
    return true;
}

bool CommandParser::IsProjectIDValid()
{
    if (IsSet("projectID")) {
        projectID = Value("projectID");
        if (CheckParamInvalidity(projectID, false)) {
            errorInfo = "Launch -projectID parameters is not match regex.";
            return false;
        }
    }
    return true;
}

bool CommandParser::IsColorModeValid()
{
    if (!IsSet("cm")) {
        return true;
    }

    string colorMode = Value("cm");
    if (colorMode != "dark" && colorMode != "light") {
        errorInfo = string("The colormode argument unsupported.");
        ELOG("Launch -cm parameters abnormal!");
        return false;
    }
    return true;
}

bool CommandParser::IsAceVersionValid()
{
    if (!IsSet("av")) {
        return true;
    }

    string aceVersion = Value("av");
    if (aceVersion != "ACE_1_0" && aceVersion != "ACE_2_0") {
        errorInfo = string("The aceVersion argument unsupported.");
        ELOG("Launch -av parameters abnormal!");
        return false;
    }
    return true;
}

bool CommandParser::IsOrientationValid()
{
    if (!IsSet("o")) {
        return true;
    }

    string orientation = Value("o");
    if (orientation != "portrait" && orientation != "landscape") {
        errorInfo = string("The orientation argument unsupported.");
        ELOG("Launch -o parameters abnormal!");
        return false;
    }
    return true;
}

bool CommandParser::IsWebSocketPortValid()
{
    if (IsSet("lws")) {
        if (CheckParamInvalidity(Value("lws"), true)) {
            errorInfo = "Launch -lws parameters is not match regex.";
            return false;
        }
        int port = atoi(Value("lws").c_str());
        if (port < MIN_PORT || port > MAX_PORT) {
            errorInfo = string("WebSocket listening port out of range: " + to_string(MIN_PORT) + "-" +
                               to_string(MAX_PORT) + ".");
            ELOG("Launch -lws parameters abnormal!");
            return false;
        }
    }
    ILOG("CommandParser WebSocket listening port: %s", Value("lws").c_str());
    return true;
}

bool CommandParser::IsScreenModeValid()
{
    string mode("dynamic");
    if (IsSet("sm")) {
        mode = Value("sm");
        if (mode != "dynamic" && mode != "static") {
            errorInfo = string("Screen picture transport mode suported: dynamic or static");
            ELOG("Launch -sm parameters abnormal!");
            return false;
        }
        screenMode = (mode == "static" ? CommandParser::ScreenMode::STATIC :
                      CommandParser::ScreenMode::DYNAMIC);
    }
    ILOG("CommandParser screen mode: %s", mode.c_str());
    return true;
}

bool CommandParser::IsLanguageValid()
{
    if (!IsSet("l")) {
        return true;
    }
    string lan = Value("pages");
    if (CheckParamInvalidity(lan, false)) {
        errorInfo = "Launch -l parameters is not match regex.";
        return false;
    }
    ILOG("CommandParser l: %s", lan.c_str());
    return true;
}

bool CommandParser::IsTracePipeNameValid()
{
    if (!IsSet("ts")) {
        return true;
    }
    string tsName = Value("ts");
    if (CheckParamInvalidity(tsName, false)) {
        errorInfo = "Launch -ts parameters is not match regex.";
        return false;
    }
    ILOG("CommandParser ts: %s", tsName.c_str());
    return true;
}

bool CommandParser::IsLocalSocketNameValid()
{
    if (!IsSet("s")) {
        return true;
    }
    string socketName = Value("s");
    if (CheckParamInvalidity(socketName, false)) {
        errorInfo = "Launch -s parameters is not match regex.";
        return false;
    }
    ILOG("CommandParser s: %s", socketName.c_str());
    return true;
}

bool CommandParser::IsConfigChangesValid()
{
    if (!IsSet("cc")) {
        return true;
    }
    string configChange = Value("cc");
    if (CheckParamInvalidity(configChange, false)) {
        ELOG("Launch -cc parameters is not match regex.");
        return false;
    }
    ILOG("CommandParser cc: %s", configChange.c_str());
    return true;
}

bool CommandParser::IsScreenDensityValid()
{
    if (!IsSet("sd")) {
        return true;
    }
    string density = Value("sd");
    if (CheckParamInvalidity(density, true)) {
        errorInfo = "Launch -sd parameters is not match regex.";
        return false;
    }
    ILOG("CommandParser sd: %s", density.c_str());
    return true;
}

bool CommandParser::IsContainerSdkPathValid()
{
    if (!IsSet("hsp")) {
        return true;
    }

    string path = Value("hsp");
    if (!FileSystem::IsDirectoryExists(path)) {
        errorInfo = string("The container sdk path does not exist.");
        ELOG("Launch -hsp parameters abnormal!");
        return false;
    }
    containerSdkPath = path;
    return true;
}

string CommandParser::HelpText()
{
    string helpText = "Usage:\n";
    for (auto index = regsHelpMap.begin(); index != regsHelpMap.end(); index++) {
        helpText += "-" + index->first + " ";
        helpText += index->second + "\n";
    }
    return helpText;
}

void CommandParser::ProcessingCommand(const std::vector<string>& strs)
{
    for (unsigned int i = 0; i < strs.size(); ++i) {
        string index = strs[i];
        auto regInfo = regsArgsCountMap.find(strs[i]);
        if (regInfo == regsArgsCountMap.end()) {
            continue;
        }

        vector<string> args;
        for (uint32_t j = 0; j < regInfo->second; ++j) {
            if (i == strs.size() - 1  || strs[i + 1][0] == '-') {
                args.push_back("");
                break;
            }
            args.push_back(strs[++i]);
        }
        argsMap[index] = args;
    }
}

int CommandParser::GetProjectModelEnumValue() const
{
    auto idxVal = std::distance(projectModels.begin(),
                                find(projectModels.begin(), projectModels.end(), projectModel));
    idxVal = (idxVal >= projectModels.size()) ? 0 : idxVal;
    return idxVal;
}

string CommandParser::GetProjectModelEnumName(int enumValue) const
{
    if (enumValue < 0 || enumValue >= projectModels.size()) {
        enumValue = 0;
    }
    return projectModels[enumValue];
}

bool CommandParser::CheckParamInvalidity(string param, bool isNum = false)
{
    regex reg(isNum ? regex4Num : regex4Str);
    return !regex_match(param.cbegin(), param.cend(), reg);
}

bool CommandParser::IsComponentModeValid()
{
    if (!IsSet("cpm")) {
        return true;
    }

    string cpm = Value("cpm");
    if (cpm != "true" && cpm != "false") {
        errorInfo = string("The component mode argument unsupported.");
        ELOG("Launch -cpm parameters abnormal!");
        return false;
    }

    isComponentMode = cpm == "true" ? true : false;
    return true;
}

bool CommandParser::IsAbilityPathValid()
{
    if (!IsSet("d")) {
        return true;
    }
    if (deviceType == "liteWearable" || deviceType == "smartVision") {
        return true;
    }
    if (!IsSet("abp")) {
        errorInfo = "Launch -d parameters without -abp parameters.";
        return false;
    }
    string path = Value("abp");
    if (path.empty()) {
        errorInfo = string("The ability path is empty.");
        ELOG("Launch -abp parameters abnormal!");
        return false;
    }
    abilityPath = path;
    return true;
}

bool CommandParser::IsStaticCardValid()
{
    if (!IsSet("staticCard")) {
        return true;
    }
    string val = Value("staticCard");
    if (val != "true" && val != "false") {
        errorInfo = string("The staticCard argument unsupported.");
        ELOG("Launch -staticCard parameters abnormal!");
        return false;
    }
    if (val == "true") {
        staticCard = true;
    }
    return true;
}

bool CommandParser::IsMainArgLengthInvalid(const char* str) const
{
    size_t argLength = strlen(str);
    if (argLength > maxMainArgLength) {
        ELOG("param size is more than %d", maxMainArgLength);
        return true;
    }
    return false;
}

bool CommandParser::IsFoldableValid()
{
    ELOG("param size is more than %d", maxMainArgLength);
    if (!IsSet("foldable")) {
        return true;
    }
    string val = Value("foldable");
    if (val != "true" && val != "false") {
        errorInfo = string("The foldable argument unsupported.");
        ELOG("Launch -foldable parameters abnormal!");
        return false;
    }
    if (val == "true") {
        foldable = true;
    }
    return true;
}

bool CommandParser::IsFoldStatusValid()
{
    if ((!IsSet("foldable")) || Value("foldable") != "true") {
        return true;
    }
    if (IsSet("foldStatus")) {
        if (Value("foldStatus") == "fold" || Value("foldStatus") == "unfold" ||
            Value("foldStatus") == "unknown" || Value("foldStatus") == "half_fold") {
            foldStatus = Value("foldStatus");
            return true;
        }
    }
    ELOG("Launch -foldStatus parameters abnormal!");
    return false;
}

bool CommandParser::IsFoldResolutionValid()
{
    if ((!IsSet("foldable")) || Value("foldable") != "true") {
        return true;
    }
    if (IsSet("fr")) {
        if (IsResolutionArgValid(string("-fr"))) {
            foldResolutionWidth = atoi(Values("-fr")[0].c_str());
            foldResolutionHeight = atoi(Values("-fr")[1].c_str());
            ILOG("CommandParser fold resolution: %d %d", foldResolutionWidth, foldResolutionHeight);
            return true;
        }
        ELOG("Launch -fr parameters abnormal!");
        return false;
    }
    ELOG("Launch -fr parameters abnormal!");
    errorInfo = string("Fold resolution must be setted.");
    return false;
}

bool CommandParser::IsFoldable() const
{
    return foldable;
}

std::string CommandParser::GetFoldStatus() const
{
    return foldStatus;
}

int32_t CommandParser::GetFoldResolutionWidth() const
{
    return foldResolutionWidth;
}

int32_t CommandParser::GetFoldResolutionHeight() const
{
    return foldResolutionHeight;
}

string CommandParser::GetLoaderJsonPath() const
{
    return loaderJsonPath;
}

bool CommandParser::IsLoaderJsonPathValid()
{
    if (!IsSet("ljPath")) {
        return true;
    }
    string path = Value("ljPath");
    if (!FileSystem::IsFileExists(path)) {
        errorInfo = string("The configuration loader.json path does not exist.");
        ELOG("Launch -ljPath parameters abnormal!");
        return false;
    }
    loaderJsonPath = path;
    return true;
}