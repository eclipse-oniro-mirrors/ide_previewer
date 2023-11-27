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

#include "CommandLine.h"

#include <algorithm>
#include <regex>
#include <sstream>

#include "CommandLineInterface.h"
#include "CommandParser.h"
#include "Interrupter.h"
#include "JsApp.h"
#include "JsAppImpl.h"
#include "JsonReader.h"
#include "LanguageManagerImpl.h"
#include "ModelConfig.h"
#include "ModelManager.h"
#include "MouseInputImpl.h"
#include "MouseWheelImpl.h"
#include "KeyInputImpl.h"
#include "PreviewerEngineLog.h"
#include "SharedData.h"
#include "VirtualMessageImpl.h"
#include "VirtualScreenImpl.h"

using namespace std;

CommandLine::CommandLine(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : args(arg), cliSocket(socket), type(commandType), commandName("")
{
    SetCommandResult("result", true);
}

CommandLine::~CommandLine()
{
    args.clear();
}

void CommandLine::CheckAndRun()
{
    if (!IsArgValid()) {
        ELOG("CheckAndRun: invalid command params");
        SetCommandResult("result", false);
        SendResult();
        return;
    }
    Run();
    SendResult();
}

void CommandLine::SendResult()
{
    if (commandResult.empty()) {
        return;
    }
    cliSocket << commandResult.toStyledString();
    commandResult.clear();
}

void CommandLine::RunAndSendResultToManager()
{
    Run();
    SendResultToManager();
}

void CommandLine::SendResultToManager()
{
    if (commandResultToManager.empty()) {
        return;
    }
    cliSocket << commandResultToManager.toStyledString();
    commandResultToManager.clear();
}

bool CommandLine::IsArgValid() const
{
    if (type == CommandType::GET) {
        return IsGetArgValid();
    }
    if (type == CommandType::SET) {
        return IsSetArgValid();
    }
    if (type == CommandType::ACTION) {
        return IsActionArgValid();
    }
    return true;
}

uint8_t CommandLine::ToUint8(std::string str) const
{
    int value = atoi(str.data());
    if (value > UINT8_MAX) {
        ELOG("CommandLine::ToUint8 value is overflow, value: %d", value);
    }
    return static_cast<uint8_t>(value);
}

void CommandLine::SetCommandName(std::string command)
{
    this->commandName = command;
}

void CommandLine::SetCommandResult(const std::string& resultType, const Json::Value& resultContent)
{
    this->commandResult["version"] = CommandLineInterface::COMMAND_VERSION;
    this->commandResult["command"] = this->commandName;
    this->commandResult[resultType] = resultContent;
}

void CommandLine::SetResultToManager(const std::string& resultType,
                                     const Json::Value& resultContent,
                                     const std::string& messageType)
{
    this->commandResultToManager["MessageType"] = messageType;
    this->commandResultToManager[resultType] = resultContent;
}

void CommandLine::Run()
{
    if (type == CommandType::GET) {
        RunGet();
    } else if (type == CommandType::SET) {
        RunSet();
    } else if (type == CommandType::ACTION) {
        RunAction();
    }
}

bool CommandLine::IsBoolType(std::string arg) const
{
    regex tofrx("^(true)|(false)$");
    if (regex_match(arg, tofrx)) {
        return true;
    }
    return false;
}

bool CommandLine::IsIntType(string arg) const
{
    regex isInt("^\\d+$");
    return regex_match(arg, isInt);
}

bool CommandLine::IsOneDigitFloatType(string arg, bool allowNegativeNumber) const
{
    if (allowNegativeNumber) {
        regex isFloat("^-?\\d+(\\.\\d+)?$");
        return regex_match(arg, isFloat);
    } else {
        regex isFloat("^\\d(\\.\\d+)?$");
        return regex_match(arg, isFloat);
    }
}

void TouchAndMouseCommand::SetEventParams(EventParams& params)
{
    if (CommandParser::GetInstance().GetScreenMode() == CommandParser::ScreenMode::STATIC) {
        return;
    }
    MouseInputImpl::GetInstance().SetMousePosition(params.x, params.y);
    MouseInputImpl::GetInstance().SetMouseStatus(params.type);
    MouseInputImpl::GetInstance().SetMouseButton(params.button);
    MouseInputImpl::GetInstance().SetMouseAction(params.action);
    MouseInputImpl::GetInstance().SetSourceType(params.sourceType);
    MouseInputImpl::GetInstance().SetSourceTool(params.sourceTool);
    MouseInputImpl::GetInstance().SetPressedBtns(params.pressedBtnsVec);
    MouseInputImpl::GetInstance().SetAxisValues(params.axisVec);
    MouseInputImpl::GetInstance().DispatchOsTouchEvent();
    std::stringstream ss;
    ss << "[";
    for (double val : params.axisVec) {
        ss << " " << val << " ";
    }
    ss << "]" << std::endl;
    ILOG("%s(%f,%f,%d,%d,%d,%d,%d,%d,%d,%s)", params.name.c_str(), params.x, params.y, params.type, params.button,
        params.action, params.sourceType, params.sourceTool, params.pressedBtnsVec.size(), params.axisVec.size(),
        ss.str().c_str());
}

bool TouchPressCommand::IsActionArgValid() const
{
    if (args.isNull() || !args.isMember("x") || !args.isMember("y") ||
        !args["x"].isInt() || !args["y"].isInt()) {
        return false;
    }
    int32_t pointX = args["x"].asInt();
    int32_t pointY = args["y"].asInt();
    if (pointX < 0 || pointX > VirtualScreenImpl::GetInstance().GetOrignalWidth()) {
        ELOG("X coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetOrignalWidth());
        return false;
    }
    if (pointY < 0 || pointY > VirtualScreenImpl::GetInstance().GetOrignalHeight()) {
        ELOG("Y coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetOrignalHeight());
        return false;
    }
    return true;
}

TouchPressCommand::TouchPressCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void TouchPressCommand::RunAction()
{
    int type = 0;
    EventParams param;
    param.x = atof(args["x"].asString().data());
    param.y = atof(args["y"].asString().data());
    param.type = type;
    param.name = "TouchPress";
    param.button = MouseInputImpl::GetInstance().defaultButton;
    param.action = MouseInputImpl::GetInstance().defaultAction;
    param.sourceType = MouseInputImpl::GetInstance().defaultSourceType;
    param.sourceTool = MouseInputImpl::GetInstance().defaultSourceTool;
    SetEventParams(param);
    SetCommandResult("result", true);
}

bool MouseWheelCommand::IsActionArgValid() const
{
    if (args.isNull() || !args.isMember("rotate") || !IsOneDigitFloatType(args["rotate"].asString(), true)) {
        return false;
    }
    return true;
}

MouseWheelCommand::MouseWheelCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void MouseWheelCommand::RunAction()
{
    if (CommandParser::GetInstance().GetScreenMode() == CommandParser::ScreenMode::STATIC) {
        return;
    }
    MouseWheelImpl::GetInstance().SetRotate(atof(args["rotate"].asString().data()));
    SetCommandResult("result", true);
    ILOG("CrownRotate (%s)", args["rotate"].asString().c_str());
}

TouchReleaseCommand::TouchReleaseCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void TouchReleaseCommand::RunAction()
{
    int type = 1;
    EventParams param;
    param.x = atof(args["x"].asString().data());
    param.y = atof(args["y"].asString().data());
    param.type = type;
    param.name = "TouchRelease";
    param.button = MouseInputImpl::GetInstance().defaultButton;
    param.action = MouseInputImpl::GetInstance().defaultAction;
    param.sourceType = MouseInputImpl::GetInstance().defaultSourceType;
    param.sourceTool = MouseInputImpl::GetInstance().defaultSourceTool;
    SetEventParams(param);
    SetCommandResult("result", true);
}

bool TouchMoveCommand::IsActionArgValid() const
{
    if (args.isNull() || !args.isMember("x") || !args.isMember("y") ||
        !args["x"].isInt() || !args["y"].isInt()) {
        return false;
    }
    int32_t pX = args["x"].asInt();
    int32_t pY = args["y"].asInt();
    if (pX < 0 || pX > VirtualScreenImpl::GetInstance().GetOrignalWidth()) {
        ELOG("X coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetOrignalWidth());
        return false;
    }
    if (pY < 0 || pY > VirtualScreenImpl::GetInstance().GetOrignalHeight()) {
        ELOG("Y coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetOrignalHeight());
        return false;
    }
    return true;
}

TouchMoveCommand::TouchMoveCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void TouchMoveCommand::RunAction()
{
    int type = 2;
    EventParams param;
    param.x = atof(args["x"].asString().data());
    param.y = atof(args["y"].asString().data());
    param.type = type;
    param.name = "TouchMove";
    param.button = MouseInputImpl::GetInstance().defaultButton;
    param.action = MouseInputImpl::GetInstance().defaultAction;
    param.sourceType = MouseInputImpl::GetInstance().defaultSourceType;
    param.sourceTool = MouseInputImpl::GetInstance().defaultSourceTool;
    SetEventParams(param);
    SetCommandResult("result", true);
}

PowerCommand::PowerCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool PowerCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("Power") || !IsOneDigitFloatType(args["Power"].asString(), false)) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    float val = args["Power"].asFloat();
    if (!SharedData<double>::IsValid(SharedDataType::BATTERY_LEVEL, val)) {
        ELOG("PowerCommand invalid value: %f", val);
        return false;
    }
    return true;
}

void PowerCommand::RunGet()
{
    double power = SharedData<double>::GetData(SharedDataType::BATTERY_LEVEL);
    Json::Value resultContent;
    resultContent["Power"] = power;
    SetCommandResult("result", resultContent);
    ILOG("Get power run finished");
}

void PowerCommand::RunSet()
{
    string power(args["Power"].asString());
    SharedData<double>::SetData(SharedDataType::BATTERY_LEVEL, atof(power.data()));
    Json::Value resultContent = true;
    SetCommandResult("result", resultContent);
    ILOG("Set power run finished, the value is: %s", args["Power"].asString().c_str());
}

VolumeCommand::VolumeCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool VolumeCommand::IsSetArgValid() const
{
    return true;
}

void VolumeCommand::RunGet()
{
    Json::Value resultContent = "Command offline";
    SetCommandResult("result", resultContent);
    ILOG("Command offline");
}

void VolumeCommand::RunSet()
{
    Json::Value resultContent = "Command offline";
    SetCommandResult("result", resultContent);
    ILOG("Command offline");
}

BarometerCommand::BarometerCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool BarometerCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("Barometer") || !args["Barometer"].isString()) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    string barometer(args["Barometer"].asString());
    if (!IsIntType(barometer)) {
        ELOG("Invalid arguments!");
        return false;
    }

    if (!SharedData<uint32_t>::IsValid(SharedDataType::PRESSURE_VALUE, static_cast<uint32_t>(atoi(barometer.data())))) {
        ELOG("Barometer invalid value: %d", atoi(barometer.data()));
        return false;
    }
    return true;
}

void BarometerCommand::RunGet()
{
    int barometer = static_cast<int>(SharedData<uint32_t>::GetData(SharedDataType::PRESSURE_VALUE));
    Json::Value resultContent;
    resultContent["Barometer"] = barometer;
    SetCommandResult("result", resultContent);
    ILOG("Get barometer run finished");
}

void BarometerCommand::RunSet()
{
    string barometer(args["Barometer"].asString());
    SharedData<uint32_t>::SetData(SharedDataType::PRESSURE_VALUE, static_cast<uint32_t>(atoi(barometer.data())));
    SetCommandResult("result", true);
    ILOG("Set barometer run finished, the value is: %s", args["Barometer"].asString().c_str());
}

ResolutionSwitchCommand::ResolutionSwitchCommand(CommandType commandType,
                                                 const Json::Value& arg,
                                                 const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool ResolutionSwitchCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("originWidth") || !args.isMember("originHeight") || !args.isMember("width") ||
        !args.isMember("height") || !args.isMember("screenDensity")) {
        ELOG("Invalid param of arguments!");
        return false;
    }
    if (!args["originWidth"].isInt() || !args["originHeight"].isInt() ||
        !args["screenDensity"].isInt() || !args["width"].isInt() || !args["height"].isInt()) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!IsIntValValid(args)) {
        return false;
    }
    return true;
}

bool ResolutionSwitchCommand::IsIntValValid(const Json::Value& args) const
{
    if (args["originWidth"].asInt() < minWidth || args["originWidth"].asInt() > maxWidth ||
        args["originHeight"].asInt() < minWidth || args["originHeight"].asInt() > maxWidth ||
        args["width"].asInt() < minWidth || args["width"].asInt() > maxWidth ||
        args["height"].asInt() < minWidth || args["height"].asInt() > maxWidth) {
        ELOG("width or height is out of range %d-%d", minWidth, maxWidth);
        return false;
    }
    if (args["screenDensity"].asInt() < minDpi || args["screenDensity"].asInt() > maxDpi) {
        ELOG("screenDensity is out of range %d-%d", minDpi, maxDpi);
        return false;
    }
    return true;
}

void ResolutionSwitchCommand::RunSet()
{
    int32_t originWidth = args["originWidth"].asInt();
    int32_t originHeight = args["originHeight"].asInt();
    int32_t width = args["width"].asInt();
    int32_t height = args["height"].asInt();
    int32_t screenDensity = args["screenDensity"].asInt();
    JsAppImpl::GetInstance().ResolutionChanged(originWidth, originHeight, width, height, screenDensity);
    SetCommandResult("result", true);
    ILOG("ResolutionSwitch run finished.");
}

OrientationCommand::OrientationCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool OrientationCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("Orientation") || !args["Orientation"].isString()) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (args["Orientation"].asString() != "portrait" && args["Orientation"].asString() != "landscape") {
        ELOG("Orientation just support [portrait,landscape].");
        return false;
    }
    return true;
}

void OrientationCommand::RunSet()
{
    std::string commandOrientation = args["Orientation"].asString();
    std::string currentOrientation = JsAppImpl::GetInstance().GetOrientation();
    if (commandOrientation != currentOrientation) {
        JsAppImpl::GetInstance().OrientationChanged(commandOrientation);
    }
    SetCommandResult("result", true);
    ILOG("Set Orientation run finished, Orientation is: %s", args["Orientation"].asString().c_str());
}

ColorModeCommand::ColorModeCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool ColorModeCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("ColorMode") || !args["ColorMode"].isString()) {
        ELOG("Invalid number of arguments!");
        return false;
    }

    if (args["ColorMode"].asString() != "light" && args["ColorMode"].asString() != "dark") {
        ELOG("ColorMode just support [light,dark]");
        return false;
    }
    return true;
}

void ColorModeCommand::RunSet()
{
    std::string commandColorMode = args["ColorMode"].asString();
    std::string currentColorMode = JsAppImpl::GetInstance().GetColorMode();
    if (commandColorMode != currentColorMode) {
        JsAppImpl::GetInstance().SetArgsColorMode(args["ColorMode"].asString());
        JsAppImpl::GetInstance().ColorModeChanged(commandColorMode);
    }
    SetCommandResult("result", true);
    ILOG("Set ColorMode run finished, ColorMode is: %s", args["ColorMode"].asString().c_str());
}

FontSelectCommand::FontSelectCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool FontSelectCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("FontSelect") || !args["FontSelect"].isBool()) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    return true;
}

void FontSelectCommand::RunSet()
{
    SetCommandResult("result", true);
    ILOG("FontSelect finished, currentSelect is: %d", args["FontSelect"].asBool());
}

MemoryRefreshCommand::MemoryRefreshCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool MemoryRefreshCommand::IsSetArgValid() const
{
    if (args.isNull()) {
        ELOG("Invalid MemoryRefresh of arguments!");
        return false;
    }
    return true;
}

void MemoryRefreshCommand::RunSet()
{
    ILOG("MemoryRefreshCommand begin.");
    bool ret = JsAppImpl::GetInstance().MemoryRefresh(args.toStyledString());
    SetCommandResult("result", ret);
    ILOG("MemoryRefresh finished.");
}

LoadDocumentCommand::LoadDocumentCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool LoadDocumentCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("url") || !args.isMember("className") || !args.isMember("previewParam") ||
        !args["url"].isString() || !args["className"].isString() || !args["previewParam"].isObject()) {
        return false;
    }
    Json::Value previewParam = args["previewParam"];
    if (!previewParam["width"].isInt() || !previewParam["height"].isInt() || !previewParam["dpi"].isInt() ||
        !previewParam["locale"].isString() || !previewParam["colorMode"].isString() ||
        !previewParam["orientation"].isString() || !previewParam["deviceType"].isString()) {
        return false;
    }
    if (!IsIntValValid(previewParam) || !IsStrValVailid(previewParam)) {
        return false;
    }
    return true;
}

bool LoadDocumentCommand::IsIntValValid(const Json::Value& previewParam) const
{
    int width = previewParam["width"].asInt();
    int height = previewParam["height"].asInt();
    int dpi = previewParam["dpi"].asInt();
    if (width < minLoadDocWidth || width > maxLoadDocWidth || height < minLoadDocWidth ||
        height > maxLoadDocWidth || dpi < minDpi || dpi > maxDpi) {
        return false;
    }
    return true;
}

bool LoadDocumentCommand::IsStrValVailid(const Json::Value& previewParam) const
{
    string locale = previewParam["locale"].asString();
    bool isLiteDevice = JsApp::IsLiteDevice(CommandParser::GetInstance().GetDeviceType());
    if (isLiteDevice) {
        if (std::find(liteSupportedLanguages.begin(), liteSupportedLanguages.end(), locale) ==
            liteSupportedLanguages.end()) {
            return false;
        }
    } else {
        if (std::find(richSupportedLanguages.begin(), richSupportedLanguages.end(), locale) ==
            richSupportedLanguages.end()) {
            return false;
        }
    }
    if (previewParam["colorMode"].asString() != "light" && previewParam["colorMode"].asString() != "dark") {
        return false;
    }
    if (previewParam["orientation"].asString() != "portrait" &&
        previewParam["orientation"].asString() != "landscape") {
        return false;
    }
    if (std::find(LoadDocDevs.begin(), LoadDocDevs.end(), previewParam["deviceType"].asString()) ==
        LoadDocDevs.end()) {
        return false;
    }
    return true;
}

void LoadDocumentCommand::RunSet()
{
    VirtualScreenImpl::GetInstance().SetLoadDocFlag(VirtualScreen::LoadDocType::START);
    ILOG("LoadDocumentCommand begin.");
    std::string pageUrl = args["url"].asString();
    std::string className = args["className"].asString();
    Json::Value previewParam = Json::nullValue;
    if (args.isMember("previewParam")) {
        previewParam = args["previewParam"];
    }
    JsAppImpl::GetInstance().LoadDocument(pageUrl, className, previewParam);
    VirtualScreenImpl::GetInstance().SetLoadDocFlag(VirtualScreen::LoadDocType::FINISHED);
    SetCommandResult("result", true);
    ILOG("LoadDocumentCommand finished.");
}

ReloadRuntimePageCommand::ReloadRuntimePageCommand(CommandType commandType,
                                                   const Json::Value& arg,
                                                   const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool ReloadRuntimePageCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("ReloadRuntimePage") || !args["ReloadRuntimePage"].isString()) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    return true;
}

void ReloadRuntimePageCommand::RunSet()
{
    std::string currentPage = args["ReloadRuntimePage"].asString();
    JsAppImpl::GetInstance().ReloadRuntimePage(currentPage);
    SetCommandResult("result", true);
    ILOG("ReloadRuntimePage finished, currentPage is: %s", args["ReloadRuntimePage"].asString().c_str());
}

CurrentRouterCommand::CurrentRouterCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void CurrentRouterCommand::RunGet()
{
    Json::Value resultContent;
    std::string currentRouter = VirtualScreenImpl::GetInstance().GetCurrentRouter();
    resultContent["CurrentRouter"] = currentRouter;
    SetResultToManager("args", resultContent, "CurrentJsRouter");
    ILOG("Get CurrentRouter run finished.");
}

LoadContentCommand::LoadContentCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void LoadContentCommand::RunGet()
{
    Json::Value resultContent;
    std::string currentRouter = VirtualScreenImpl::GetInstance().GetAbilityCurrentRouter();
    resultContent["AbilityCurrentRouter"] = currentRouter;
    SetResultToManager("args", resultContent, "AbilityCurrentJsRouter");
    ILOG("Get AbilityCurrentRouter run finished.");
}

LanguageCommand::LanguageCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool LanguageCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("Language") || !args["Language"].isString()) {
        ELOG("Invalid number of arguments!");
        return false;
    }

    CommandParser& cmdParser = CommandParser::GetInstance();
    string deviceType = cmdParser.GetDeviceType();
    bool isLiteDevice = JsApp::IsLiteDevice(deviceType);
    if (isLiteDevice) {
        if (std::find(liteSupportedLanguages.begin(), liteSupportedLanguages.end(), args["Language"].asString()) ==
            liteSupportedLanguages.end()) {
            ELOG("Language not support liteDevice : %s", args["Language"].asString().c_str());
            return false;
        }
    } else {
        if (std::find(richSupportedLanguages.begin(), richSupportedLanguages.end(), args["Language"].asString()) ==
            richSupportedLanguages.end()) {
            ELOG("Language not support richDevice : %s", args["Language"].asString().c_str());
            return false;
        }
    }
    return true;
}

void LanguageCommand::RunGet()
{
    std::string language = SharedData<string>::GetData(SharedDataType::LANGUAGE);
    Json::Value resultContent;
    resultContent["Language"] = language;
    SetCommandResult("result", resultContent);
    ILOG("Get language run finished.");
}

void LanguageCommand::RunSet()
{
    string language(args["Language"].asString());
    SharedData<string>::SetData(SharedDataType::LANGUAGE, language);
    SetCommandResult("result", true);
    ILOG("Set language run finished, language is: %s", language.c_str());
}

SupportedLanguagesCommand::SupportedLanguagesCommand(CommandType commandType,
                                                     const Json::Value& arg,
                                                     const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void SupportedLanguagesCommand::RunGet()
{
    Json::Value resultContent;
    Json::Value languageList;
    string deviceType = CommandParser::GetInstance().GetDeviceType();
    bool isLiteDevice = JsApp::IsLiteDevice(deviceType);
    if (!deviceType.empty() && !isLiteDevice) {
        for (auto iter = richSupportedLanguages.begin(); iter != richSupportedLanguages.end(); iter++) {
            languageList.append(*iter);
        }
    } else {
        for (auto iter = liteSupportedLanguages.begin(); iter != liteSupportedLanguages.end(); iter++) {
            languageList.append(*iter);
        }
    }
    resultContent["SupportedLanguages"] = languageList;
    SetCommandResult("result", resultContent);
    ILOG("Get supportedLanguages run finished.");
}

LocationCommand::LocationCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool LocationCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("latitude") || !args.isMember("longitude")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    string latitude(args["latitude"].asString());
    string longitude(args["longitude"].asString());
    regex isDob("^([\\-]*[0-9]{1,}[\\.][0-9]*)$");
    if (!regex_match(latitude, isDob) || !regex_match(longitude, isDob)) {
        ELOG("Invalid arguments!");
        return false;
    }

    if (!SharedData<double>::IsValid(SharedDataType::LATITUDE, atof(latitude.data()))) {
        ELOG("LocationCommand invalid latitude value: %f", atof(latitude.data()));
        return false;
    }

    if (!SharedData<double>::IsValid(SharedDataType::LONGITUDE, atof(longitude.data()))) {
        ELOG("LocationCommand invalid longitude value: %f", atof(longitude.data()));
        return false;
    }
    return true;
}

void LocationCommand::RunGet()
{
    double longitude = SharedData<double>::GetData(SharedDataType::LONGITUDE);
    double latitude = SharedData<double>::GetData(SharedDataType::LATITUDE);
    Json::Value resultContent;
    resultContent["latitude"] = latitude;
    resultContent["longitude"] = longitude;
    SetCommandResult("result", resultContent);
    ILOG("Get location run finished");
}

void LocationCommand::RunSet()
{
    string latitude(args["latitude"].asString());
    string longitude(args["longitude"].asString());
    SharedData<double>::SetData(SharedDataType::LONGITUDE, atof(longitude.data()));
    SharedData<double>::SetData(SharedDataType::LATITUDE, atof(latitude.data()));
    Json::Value resultContent = true;
    SetCommandResult("result", resultContent);
    ILOG("Set location run finished, latitude: %s,longitude: %s", latitude.c_str(), longitude.c_str());
}

DistributedCommunicationsCommand::DistributedCommunicationsCommand(CommandType commandType,
                                                                   const Json::Value& arg,
                                                                   const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void DistributedCommunicationsCommand::RunAction()
{
    MessageInfo info;
    info.deviceID = args["DeviceId"].asString();
    info.bundleName = args["bundleName"].asString();
    info.abilityName = args["abilityName"].asString();
    info.message = args["message"].asString();
    VirtualMessageImpl::GetInstance().SendVirtualMessage(info);
    Json::Value resultContent = true;
    SetCommandResult("result", resultContent);
    ILOG("Send distributedCommunications run finished");
}

bool DistributedCommunicationsCommand::IsActionArgValid() const
{
    if (args.isNull() || !args.isMember("DeviceId") || !args.isMember("bundleName") || !args.isMember("abilityName") ||
        !args.isMember("message")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (args["DeviceId"].asString().empty() || args["bundleName"].asString().empty() ||
        args["abilityName"].asString().empty() || args["message"].asString().empty()) {
        ELOG("Invalid arguments!");
        return false;
    }
    return true;
}

std::vector<char> DistributedCommunicationsCommand::StringToCharVector(string str) const
{
    vector<char> vec(str.begin(), str.end());
    vec.push_back('\0');
    return vec;
}

KeepScreenOnStateCommand::KeepScreenOnStateCommand(CommandType commandType,
                                                   const Json::Value& arg,
                                                   const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void KeepScreenOnStateCommand::RunGet()
{
    Json::Value result;
    result["KeepScreenOnState"] = SharedData<bool>::GetData(SharedDataType::KEEP_SCREEN_ON);
    SetCommandResult("result", result);
    ILOG("Get keepScreenOnState run finished");
}

void KeepScreenOnStateCommand::RunSet()
{
    SharedData<bool>::SetData(SharedDataType::KEEP_SCREEN_ON, args["KeepScreenOnState"].asString() == "true");
    Json::Value result = true;
    SetCommandResult("result", result);
    ILOG("Set keepScreenOnState run finished, the value is: %s", args["KeepScreenOnState"].asString().c_str());
}

bool KeepScreenOnStateCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("KeepScreenOnState")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!IsBoolType(args["KeepScreenOnState"].asString())) {
        ELOG("arg 0: %s", args["KeepScreenOnState"].asString().c_str());
        return false;
    }
    return true;
}

WearingStateCommand::WearingStateCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void WearingStateCommand::RunGet()
{
    Json::Value result;
    result["WearingState"] = SharedData<bool>::GetData(SharedDataType::WEARING_STATE);
    SetCommandResult("result", result);
    ILOG("Get wearingState run finished");
}

void WearingStateCommand::RunSet()
{
    SharedData<bool>::SetData(SharedDataType::WEARING_STATE, args["WearingState"].asString() == "true");
    Json::Value result = true;
    SetCommandResult("result", result);
    ILOG("Set wearingState run finished, the value is: %s", args["WearingState"].asString().c_str());
}

bool WearingStateCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("WearingState")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!IsBoolType(args["WearingState"].asString())) {
        ILOG("arg 0: %s", args["WearingState"].asString().c_str());
        return false;
    }
    return true;
}

BrightnessModeCommand::BrightnessModeCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void BrightnessModeCommand::RunGet()
{
    Json::Value result;
    result["BrightnessMode"] = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_MODE);
    SetCommandResult("result", result);
    ILOG("Get brightnessMode run finished");
}

void BrightnessModeCommand::RunSet()
{
    SharedData<uint8_t>::SetData(SharedDataType::BRIGHTNESS_MODE,
                                 static_cast<uint8_t>(atoi(args["BrightnessMode"].asString().data())));
    Json::Value result = true;
    SetCommandResult("result", result);
    ILOG("Set brightnessMode run finished, the value is: %s", args["BrightnessMode"].asString().c_str());
}

bool BrightnessModeCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("BrightnessMode")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!std::regex_match(args["BrightnessMode"].asString(), std::regex("^\\d$"))) {
        ELOG("regex match error");
        return false;
    }
    uint8_t temp = ToUint8(args["BrightnessMode"].asString());
    if (!SharedData<uint8_t>::IsValid(SharedDataType::BRIGHTNESS_MODE, temp)) {
        ELOG("BrightnessModeCommand invalid value: %d", temp);
        return false;
    }
    return true;
}

ChargeModeCommand::ChargeModeCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void ChargeModeCommand::RunGet()
{
    Json::Value result;
    result["ChargeMode"] = SharedData<uint8_t>::GetData(SharedDataType::BATTERY_STATUS);
    SetCommandResult("result", result);
    ILOG("Get chargeMode run finished");
}

void ChargeModeCommand::RunSet()
{
    SharedData<uint8_t>::SetData(SharedDataType::BATTERY_STATUS,
                                 static_cast<uint8_t>(atoi(args["ChargeMode"].asString().data())));
    Json::Value result = true;
    SetCommandResult("result", result);
    ILOG("Set chargeMode run finished, the value is: %s", args["ChargeMode"].asString().c_str());
}

bool ChargeModeCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("ChargeMode")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!std::regex_match(args["ChargeMode"].asString().data(), std::regex("\\d"))) {
        ELOG("Invalid arguments!");
        return false;
    }
    uint8_t temp = ToUint8(args["ChargeMode"].asString());
    if (!SharedData<uint8_t>::IsValid(SharedDataType::BATTERY_STATUS, temp)) {
        ELOG("ChargeModeCommand invalid value: %d", temp);
        return false;
    }
    return true;
}

BrightnessCommand::BrightnessCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void BrightnessCommand::RunGet()
{
    Json::Value result;
    result["Brightness"] = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_VALUE);
    SetCommandResult("result", result);
    ILOG("Get brightness run finished");
}

void BrightnessCommand::RunSet()
{
    SharedData<uint8_t>::SetData(SharedDataType::BRIGHTNESS_VALUE,
                                 static_cast<uint8_t>(atoi(args["Brightness"].asString().data())));
    Json::Value result = true;
    SetCommandResult("result", result);
    ILOG("Set brightness run finished, the value is: %s", args["Brightness"].asString().c_str());
}

bool BrightnessCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("Brightness")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!std::regex_match(args["Brightness"].asString().data(), std::regex("\\d+"))) {
        ELOG("Invalid arguments!");
        return false;
    }
    uint8_t temp = ToUint8(args["Brightness"].asString());
    if (!SharedData<uint8_t>::IsValid(SharedDataType::BRIGHTNESS_VALUE, temp)) {
        ELOG("BrightnessCommand invalid value: ", temp);
        return false;
    }
    return true;
}

HeartRateCommand::HeartRateCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void HeartRateCommand::RunGet()
{
    Json::Value result;
    result["HeartRate"] = SharedData<uint8_t>::GetData(SharedDataType::HEARTBEAT_VALUE);
    SetCommandResult("result", result);
    ILOG("Get heartRate run finished");
}

void HeartRateCommand::RunSet()
{
    SharedData<uint8_t>::SetData(SharedDataType::HEARTBEAT_VALUE,
                                 static_cast<uint8_t>(atoi(args["HeartRate"].asString().data())));
    Json::Value result = true;
    SetCommandResult("result", result);
    ILOG("Set heartRate run finished, the value is: %s", args["HeartRate"].asString().c_str());
}

bool HeartRateCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("HeartRate")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!std::regex_match(args["HeartRate"].asString().data(), std::regex("\\d+"))) {
        ELOG("Invalid arguments!");
        return false;
    }
    if (atoi(args["HeartRate"].asString().data()) > UINT8_MAX) {
        ELOG("Invalid arguments!");
        return false;
    }
    uint8_t temp = ToUint8(args["HeartRate"].asString());
    if (!SharedData<uint8_t>::IsValid(SharedDataType::HEARTBEAT_VALUE, temp)) {
        ELOG("HeartRateCommand invalid value: %d", temp);
        return false;
    }
    return true;
}

StepCountCommand::StepCountCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void StepCountCommand::RunGet()
{
    Json::Value result;
    result["StepCount"] = SharedData<uint32_t>::GetData(SharedDataType::SUMSTEP_VALUE);
    SetCommandResult("result", result);
    ILOG("Get stepCount run finished");
}

void StepCountCommand::RunSet()
{
    SharedData<uint32_t>::SetData(SharedDataType::SUMSTEP_VALUE,
                                  static_cast<uint32_t>(atoi(args["StepCount"].asString().data())));
    Json::Value result = true;
    SetCommandResult("result", result);
    ILOG("Set stepCount run finished, the value is: %s", args["StepCount"].asString().c_str());
}

bool StepCountCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("StepCount")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!std::regex_match(args["StepCount"].asString().data(), std::regex("\\d+"))) {
        ELOG("Invalid arguments!");
        return false;
    }

    uint32_t temp = ToUint8(args["StepCount"].asString());
    if (!SharedData<uint32_t>::IsValid(SharedDataType::SUMSTEP_VALUE, temp)) {
        ELOG("StepCountCommand invalid value: %d", temp);
        return false;
    }
    return true;
}

InspectorJSONTree::InspectorJSONTree(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void InspectorJSONTree::RunAction()
{
    ILOG("GetJsonTree run!");
    std::string str = JsAppImpl::GetInstance().GetJSONTree();
    if (str == "null") {
        str = "{\"children\":\"empty json tree\"}";
    }
    SetCommandResult("result", str);
    ILOG("SendJsonTree end!");
}

InspectorDefault::InspectorDefault(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void InspectorDefault::RunAction()
{
    ILOG("GetDefaultJsonTree run!");
    std::string str = JsAppImpl::GetInstance().GetDefaultJSONTree();
    SetCommandResult("result", str);
    ILOG("SendDefaultJsonTree end!");
}

ExitCommand::ExitCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void ExitCommand::RunAction()
{
    ILOG("ExitCommand run.");
    Json::Value res = true;
    SetCommandResult("result", res);
    SendResult();
    Interrupter::Interrupt();
    ILOG("Ready to exit");
}

DeviceTypeCommand::DeviceTypeCommand(CommandLine::CommandType commandType,
                                     const Json::Value& arg,
                                     const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void DeviceTypeCommand::RunSet() {}

ResolutionCommand::ResolutionCommand(CommandLine::CommandType commandType,
                                     const Json::Value& arg,
                                     const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void ResolutionCommand::RunSet() {}

BackClickedCommand::BackClickedCommand(CommandLine::CommandType commandType,
                                       const Json::Value& arg,
                                       const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void BackClickedCommand::RunAction()
{
    MouseInputImpl::GetInstance().DispatchOsBackEvent();
    ILOG("BackClickCommand run");
    Json::Value res = true;
    SetCommandResult("result", res);
    ILOG("BackClickCommand end");
}

RestartCommand::RestartCommand(CommandLine::CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void RestartCommand::RunAction()
{
    ILOG("RestartCommand start");
    JsAppImpl::GetInstance().Restart();
    Json::Value res = true;
    SetCommandResult("result", res);
    ILOG("RestartCommand end");
}

FastPreviewMsgCommand::FastPreviewMsgCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void FastPreviewMsgCommand::RunGet()
{
    Json::Value resultContent;
    std::string fastPreviewMsg = VirtualScreenImpl::GetInstance().GetFastPreviewMsg();
    resultContent["FastPreviewMsg"] = fastPreviewMsg;
    SetResultToManager("args", resultContent, "MemoryRefresh");
    ILOG("Get FastPreviewMsgCommand run finished.");
}

DropFrameCommand::DropFrameCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool DropFrameCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("frequency") || !args["frequency"].isInt()) {
        ELOG("Invalid DropFrame of arguments!");
        return false;
    }
    if (args["frequency"].asInt() < 0) {
        ELOG("DropFrame param frequency must greater than or equal to 0");
        return false;
    }
    return true;
}

void DropFrameCommand::RunSet()
{
    ILOG("Set DropFrame frequency start.");
    int frequency = args["frequency"].asInt();
    VirtualScreenImpl::GetInstance().SetDropFrameFrequency(frequency);
    SetCommandResult("result", true);
    ILOG("Set DropFrame frequency: %sms.", frequency);
}

bool KeyPressCommand::IsActionArgValid() const
{
    if (args.isNull() || !args.isMember("isInputMethod") || !args["isInputMethod"].isBool()) {
        ELOG("Param isInputMethod's value is invalid.");
        return false;
    }
    bool isInputMethod = args["isInputMethod"].asBool();
    if (isInputMethod) {
        return IsImeArgsValid();
    } else {
        return IsKeyArgsValid();
    }
}

bool KeyPressCommand::IsImeArgsValid() const
{
    if (!args.isMember("codePoint") || !args["codePoint"].isInt()) {
        ELOG("Param codePoint's value is invalid.");
        return false;
    }
    return true;
}

bool KeyPressCommand::IsKeyArgsValid() const
{
    if (!args.isMember("keyCode") || !args["keyCode"].isInt() || !args["keyAction"].isInt() ||
        !args.isMember("keyAction") || !args["keyAction"].isInt() ||
        !args.isMember("pressedCodes") || !args["pressedCodes"].isArray() ||
        args["pressedCodes"].size() < 1 || (args.isMember("keyString") && !args["keyString"].isString())) {
        ELOG("Param keyEvent's value is invalid.");
        return false;
    }
    if (!args.isMember("keyString")) {
        ILOG("Param keyString is lost, it will be empty string.");
    }
    if (args["keyAction"].asInt() < minActionVal || args["keyAction"].asInt() > maxActionVal) {
        ELOG("Param keyAction's value is invalid,value range %d-%d.", minActionVal, maxActionVal);
        return false;
    }
    int keyCode = args["keyCode"].asInt();
    if (keyCode > maxKeyVal || keyCode < minKeyVal) {
        ELOG("Param pressedCode value is invalid,value range %d-%d.", minKeyVal, maxKeyVal);
        return false;
    }
    Json::Value arrayNum = args["pressedCodes"];
    for (unsigned int i = 0; i < arrayNum.size(); i++) {
        if (!arrayNum[i].isInt()) {
            ELOG("Param pressedCodes's value is invalid.");
            return false;
        }
        int pressedCode = arrayNum[i].asInt();
        if (pressedCode > maxKeyVal || pressedCode < minKeyVal) {
            ELOG("Param pressedCode value is invalid,value range %d-%d.", minKeyVal, maxKeyVal);
            return false;
        }
    }
    return true;
}

KeyPressCommand::KeyPressCommand(CommandType commandType, const Json::Value& arg,
                                 const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void KeyPressCommand::RunAction()
{
    if (CommandParser::GetInstance().GetScreenMode() == CommandParser::ScreenMode::STATIC) {
        return;
    }
    bool isInputMethod = args["isInputMethod"].asBool();
    if (isInputMethod) {
        VirtualScreen::inputMethodCountPerMinute++;
        unsigned int codePoint = args["codePoint"].asInt();
        KeyInputImpl::GetInstance().SetCodePoint(codePoint);
        KeyInputImpl::GetInstance().DispatchOsInputMethodEvent();
    } else {
        VirtualScreen::inputKeyCountPerMinute++;
        int32_t keyCode = args["keyCode"].asInt();
        int32_t keyAction = args["keyAction"].asInt();
        Json::Value pressedCodes = args["pressedCodes"];
        vector<int32_t> pressedCodesVec;
        for (unsigned int i = 0; i < pressedCodes.size(); i++) {
            pressedCodesVec.push_back(pressedCodes[i].asInt());
        }
        string keyString = "";
        if (args.isMember("keyString") && args["keyString"].isString()) {
            keyString = args["keyString"].asString();
        }
        KeyInputImpl::GetInstance().SetKeyEvent(keyCode, keyAction, pressedCodesVec, keyString);
        KeyInputImpl::GetInstance().DispatchOsKeyEvent();
    }
    SetCommandResult("result", true);
    ILOG("KeyPressCommand run finished.");
}

bool PointEventCommand::IsActionArgValid() const
{
    return IsArgsExist() && IsArgsValid();
}

bool PointEventCommand::IsArgsExist() const
{
    if (args.isNull() || !args.isMember("x") || !args.isMember("y") ||
        !args["x"].isInt() || !args["y"].isInt()) {
        return false;
    }
    if (!args.isMember("button") || !args.isMember("action") ||
        !args["button"].isInt() || !args["action"].isInt()) {
        return false;
    }
    if (!args.isMember("sourceType") || !args.isMember("sourceTool") ||
        !args["sourceType"].isInt() || !args["sourceTool"].isInt()) {
        return false;
    }
    if (!args.isMember("axisValues") || !args["axisValues"].isArray()) {
        return false;
    }
    return true;
}

bool PointEventCommand::IsArgsValid() const
{
    int32_t pointX = args["x"].asInt();
    int32_t pointY = args["y"].asInt();
    int32_t button = args["button"].asInt();
    int32_t action = args["action"].asInt();
    int32_t sourceType = args["sourceType"].asInt();
    int32_t sourcceTool = args["sourcceTool"].asInt();
    if (pointX < 0 || pointX > VirtualScreenImpl::GetInstance().GetOrignalWidth()) {
        ELOG("X coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetOrignalWidth());
        return false;
    }
    if (pointY < 0 || pointY > VirtualScreenImpl::GetInstance().GetOrignalHeight()) {
        ELOG("Y coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetOrignalHeight());
        return false;
    }
    if (button < -1 || action < 0 || sourceType < 0 || sourcceTool < 0) {
        ELOG("action,sourceType,sourcceTool must >= 0, button must >= -1");
        return false;
    }
    Json::Value axisArrayNum = args["axisValues"];
    for (unsigned int i = 0; i < axisArrayNum.size(); i++) {
        if (!axisArrayNum[i].isDouble()) {
            ELOG("Param axisValues's value is invalid.");
            return false;
        }
    }
    return true;
}

PointEventCommand::PointEventCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void PointEventCommand::RunAction()
{
    int type = 9;
    EventParams param;
    if (args.isMember("pressedButtons") && args["pressedButtons"].isArray()) {
        Json::Value pressedCodes = args["pressedCodes"];
        for (unsigned int i = 0; i < pressedCodes.size(); i++) {
            if (!pressedCodes[i].isInt() || pressedCodes[i].asInt() < -1) {
                continue;
            }
            param.pressedBtnsVec.insert(pressedCodes[i].asInt());
        }
    }
    vector<double> axisVec; // 13 is array size
    Json::Value axisCodes = args["axisValues"];
    for (unsigned int i = 0; i < axisCodes.size(); i++) {
        param.axisVec.push_back(axisCodes[i].asDouble());
    }
    param.x = atof(args["x"].asString().data());
    param.y = atof(args["y"].asString().data());
    param.type = type;
    param.button = args["button"].asInt();
    param.action = args["action"].asInt();
    param.sourceType = args["sourceType"].asInt();
    param.sourceTool = args["sourceTool"].asInt();
    param.name = "PointEvent";
    SetEventParams(param);
    SetCommandResult("result", true);
}

FoldStatusCommand::FoldStatusCommand(CommandType commandType, const Json::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool FoldStatusCommand::IsSetArgValid() const
{
    if (args.isNull() || !args.isMember("FoldStatus") || !args["FoldStatus"].isString()) {
        ELOG("Invalid FoldStatus of arguments!");
        return false;
    }
    if (args["FoldStatus"].asString() == "fold" || args["FoldStatus"].asString() == "unfold" ||
        args["FoldStatus"].asString() == "unknown" || args["FoldStatus"].asString() == "half_fold") {
        return true;
    }
    ELOG("FoldStatus param must be \"fold\" or \"unfold\" or \"unknown\" or \"half_fold\"");
    return false;
}

void FoldStatusCommand::RunSet()
{
    std::string commandStatus = args["FoldStatus"].asString();
    std::string currentStatus = VirtualScreenImpl::GetInstance().GetFoldStatus();
    if (commandStatus != currentStatus) {
        JsAppImpl::GetInstance().FoldStatusChanged(commandStatus);
    }
    SetCommandResult("result", true);
    ILOG("Set FoldStatus run finished, FoldStatus is: %s", args["FoldStatus"].asString().c_str());
}