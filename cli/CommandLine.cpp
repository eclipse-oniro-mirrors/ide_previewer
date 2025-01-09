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
#include <chrono>
#include <ctime>
#include <iomanip>

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

CommandLine::CommandLine(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : args(arg), cliSocket(socket), type(commandType), commandName("")
{
}

CommandLine::~CommandLine()
{
}

void CommandLine::CheckAndRun()
{
    if (!IsArgValid()) {
        ELOG("CheckAndRun: invalid command params");
        SetCommandResult("result", JsonReader::CreateBool(false));
        SendResult();
        return;
    }
    Run();
    SendResult();
}

void CommandLine::SendResult()
{
    if (commandResult.IsNull() || !commandResult.IsValid()) {
        return;
    }
    cliSocket << commandResult.ToStyledString();
    ELOG("SendResult commandResult: %s", commandResult.ToStyledString().c_str());
    commandResult.Clear();
}

void CommandLine::RunAndSendResultToManager()
{
    Run();
    SendResultToManager();
}

void CommandLine::SendResultToManager()
{
    if (commandResultToManager.IsNull() || !commandResultToManager.IsValid()) {
        return;
    }
    cliSocket << commandResultToManager.ToStyledString();
    commandResultToManager.Clear();
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

void CommandLine::SetCommandResult(const std::string& resultType, const Json2::Value& resultContent)
{
    this->commandResult.Add("version", CommandLineInterface::COMMAND_VERSION.c_str());
    this->commandResult.Add("command", this->commandName.c_str());
    this->commandResult.Add(resultType.c_str(), resultContent);
}

void CommandLine::SetResultToManager(const std::string& resultType,
                                     const Json2::Value& resultContent,
                                     const std::string& messageType)
{
    this->commandResultToManager.Add("MessageType", messageType.c_str());
    this->commandResultToManager.Add(resultType.c_str(), resultContent);
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
    std::regex tofrx("^(true)|(false)$");
    if (regex_match(arg, tofrx)) {
        return true;
    }
    return false;
}

bool CommandLine::IsIntType(std::string arg) const
{
    std::regex isInt("^\\d+$");
    return regex_match(arg, isInt);
}

bool CommandLine::IsOneDigitFloatType(std::string arg, bool allowNegativeNumber) const
{
    if (allowNegativeNumber) {
        std::regex isFloat("^-?\\d+(\\.\\d+)?$");
        return regex_match(arg, isFloat);
    } else {
        std::regex isFloat("^\\d(\\.\\d+)?$");
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
    if (args.IsNull() || !args.IsMember("x") || !args.IsMember("y") ||
        !args["x"].IsInt() || !args["y"].IsInt()) {
        return false;
    }
    int32_t pointX = args["x"].AsInt();
    int32_t pointY = args["y"].AsInt();
    if (pointX < 0 || pointX > VirtualScreenImpl::GetInstance().GetCurrentWidth()) {
        ELOG("X coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetCurrentWidth());
        return false;
    }
    if (pointY < 0 || pointY > VirtualScreenImpl::GetInstance().GetCurrentHeight()) {
        ELOG("Y coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetCurrentHeight());
        return false;
    }
    return true;
}

TouchPressCommand::TouchPressCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void TouchPressCommand::RunAction()
{
    int type = 0;
    EventParams param;
    param.x = args["x"].AsDouble();
    param.y = args["y"].AsDouble();
    param.type = type;
    param.name = "TouchPress";
    param.button = MouseInputImpl::GetInstance().defaultButton;
    param.action = MouseInputImpl::GetInstance().defaultAction;
    param.sourceType = MouseInputImpl::GetInstance().defaultSourceType;
    param.sourceTool = MouseInputImpl::GetInstance().defaultSourceTool;
    SetEventParams(param);
    SetCommandResult("result", JsonReader::CreateBool(true));
}

bool MouseWheelCommand::IsActionArgValid() const
{
    if (args.IsNull() || !args.IsMember("rotate") || !args["rotate"].IsDouble()) {
        return false;
    }
    return true;
}

MouseWheelCommand::MouseWheelCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void MouseWheelCommand::RunAction()
{
    if (CommandParser::GetInstance().GetScreenMode() == CommandParser::ScreenMode::STATIC) {
        return;
    }
    MouseWheelImpl::GetInstance().SetRotate(args["rotate"].AsDouble());
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("CrownRotate (%f)", args["rotate"].AsDouble());
}

bool TouchReleaseCommand::IsActionArgValid() const
{
    if (args.IsNull() || !args.IsMember("x") || !args.IsMember("y") ||
        !args["x"].IsInt() || !args["y"].IsInt()) {
        return false;
    }
    int32_t pX = args["x"].AsInt();
    int32_t pY = args["y"].AsInt();
    if (pY < 0 || pY > VirtualScreenImpl::GetInstance().GetCurrentHeight()) {
        ELOG("Y coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetCurrentHeight());
        return false;
    }
    if (pX < 0 || pX > VirtualScreenImpl::GetInstance().GetCurrentWidth()) {
        ELOG("X coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetCurrentWidth());
        return false;
    }
    return true;
}

TouchReleaseCommand::TouchReleaseCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void TouchReleaseCommand::RunAction()
{
    int type = 1;
    EventParams param;
    param.x = args["x"].AsDouble();
    param.y = args["y"].AsDouble();
    param.type = type;
    param.name = "TouchRelease";
    param.button = MouseInputImpl::GetInstance().defaultButton;
    param.action = MouseInputImpl::GetInstance().defaultAction;
    param.sourceType = MouseInputImpl::GetInstance().defaultSourceType;
    param.sourceTool = MouseInputImpl::GetInstance().defaultSourceTool;
    SetEventParams(param);
    SetCommandResult("result", JsonReader::CreateBool(true));
}

bool TouchMoveCommand::IsActionArgValid() const
{
    if (args.IsNull() || !args.IsMember("x") || !args.IsMember("y") ||
        !args["x"].IsInt() || !args["y"].IsInt()) {
        return false;
    }
    int32_t pX = args["x"].AsInt();
    int32_t pY = args["y"].AsInt();
    if (pX < 0 || pX > VirtualScreenImpl::GetInstance().GetCurrentWidth()) {
        ELOG("X coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetCurrentWidth());
        return false;
    }
    if (pY < 0 || pY > VirtualScreenImpl::GetInstance().GetCurrentHeight()) {
        ELOG("Y coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetCurrentHeight());
        return false;
    }
    return true;
}

TouchMoveCommand::TouchMoveCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void TouchMoveCommand::RunAction()
{
    int type = 2;
    EventParams param;
    param.x = args["x"].AsDouble();
    param.y = args["y"].AsDouble();
    param.type = type;
    param.name = "TouchMove";
    param.button = MouseInputImpl::GetInstance().defaultButton;
    param.action = MouseInputImpl::GetInstance().defaultAction;
    param.sourceType = MouseInputImpl::GetInstance().defaultSourceType;
    param.sourceTool = MouseInputImpl::GetInstance().defaultSourceTool;
    SetEventParams(param);
    SetCommandResult("result", JsonReader::CreateBool(true));
}

PowerCommand::PowerCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool PowerCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("Power") || !args["Power"].IsDouble()) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    double val = args["Power"].AsDouble();
    if (!SharedData<double>::IsValid(SharedDataType::BATTERY_LEVEL, val)) {
        ELOG("PowerCommand invalid value: %f", val);
        return false;
    }
    return true;
}

void PowerCommand::RunGet()
{
    double power = SharedData<double>::GetData(SharedDataType::BATTERY_LEVEL);
    Json2::Value resultContent = JsonReader::CreateObject();
    resultContent.Add("Power", power);
    SetCommandResult("result", resultContent);
    ILOG("Get power run finished");
}

void PowerCommand::RunSet()
{
    double val = args["Power"].AsDouble();
    SharedData<double>::SetData(SharedDataType::BATTERY_LEVEL, val);
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set power run finished, the value is: %f", val);
}

VolumeCommand::VolumeCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool VolumeCommand::IsSetArgValid() const
{
    return true;
}

void VolumeCommand::RunGet()
{
    SetCommandResult("result", JsonReader::CreateString("Command offline"));
    ILOG("Command offline");
}

void VolumeCommand::RunSet()
{
    SetCommandResult("result", JsonReader::CreateString("Command offline"));
    ILOG("Command offline");
}

BarometerCommand::BarometerCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool BarometerCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("Barometer") || !args["Barometer"].IsUInt()) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    uint32_t val = args["Barometer"].AsUInt();
    if (!SharedData<uint32_t>::IsValid(SharedDataType::PRESSURE_VALUE, val)) {
        ELOG("Barometer invalid value: %d", val);
        return false;
    }
    return true;
}

void BarometerCommand::RunGet()
{
    int barometer = static_cast<int>(SharedData<uint32_t>::GetData(SharedDataType::PRESSURE_VALUE));
    Json2::Value resultContent = JsonReader::CreateObject();
    resultContent.Add("Barometer", barometer);
    SetCommandResult("result", resultContent);
    ILOG("Get barometer run finished");
}

void BarometerCommand::RunSet()
{
    uint32_t val = args["Barometer"].AsUInt();
    SharedData<uint32_t>::SetData(SharedDataType::PRESSURE_VALUE, val);
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set barometer run finished, the value is: %d", val);
}

ResolutionSwitchCommand::ResolutionSwitchCommand(CommandType commandType,
                                                 const Json2::Value& arg,
                                                 const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool ResolutionSwitchCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("originWidth") || !args.IsMember("originHeight") || !args.IsMember("width") ||
        !args.IsMember("height") || !args.IsMember("screenDensity")) {
        ELOG("Invalid param of arguments!");
        return false;
    }
    if (!args["originWidth"].IsInt() || !args["originHeight"].IsInt() ||
        !args["screenDensity"].IsInt() || !args["width"].IsInt() || !args["height"].IsInt()) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!IsIntValValid()) {
        return false;
    }
    if (args.IsMember("reason")) {
        if (!args["reason"].IsString()) {
            return false;
        }
        std::string reason = args["reason"].AsString();
        if (reason != "rotation" && reason != "resize" && reason != "undefined") {
            ELOG("Invalid value of reason!");
            return false;
        }
    }
    return true;
}

bool ResolutionSwitchCommand::IsIntValValid() const
{
    if (args["originWidth"].AsInt() < minWidth || args["originWidth"].AsInt() > maxWidth ||
        args["originHeight"].AsInt() < minWidth || args["originHeight"].AsInt() > maxWidth ||
        args["width"].AsInt() < minWidth || args["width"].AsInt() > maxWidth ||
        args["height"].AsInt() < minWidth || args["height"].AsInt() > maxWidth) {
        ELOG("width or height is out of range %d-%d", minWidth, maxWidth);
        return false;
    }
    if (args["screenDensity"].AsInt() < minDpi || args["screenDensity"].AsInt() > maxDpi) {
        ELOG("screenDensity is out of range %d-%d", minDpi, maxDpi);
        return false;
    }
    return true;
}

void ResolutionSwitchCommand::RunSet()
{
    int32_t originWidth = args["originWidth"].AsInt();
    int32_t originHeight = args["originHeight"].AsInt();
    int32_t width = args["width"].AsInt();
    int32_t height = args["height"].AsInt();
    int32_t screenDensity = args["screenDensity"].AsInt();
    std::string reason = "undefined";
    if (args.IsMember("reason")) {
        reason = args["reason"].AsString();
    }
    ResolutionParam param(originWidth, originHeight, width, height);
    JsAppImpl::GetInstance().ResolutionChanged(param, screenDensity, reason);
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("ResolutionSwitch run finished.");
}

OrientationCommand::OrientationCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool OrientationCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("Orientation") || !args["Orientation"].IsString()) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (args["Orientation"].AsString() != "portrait" && args["Orientation"].AsString() != "landscape") {
        ELOG("Orientation just support [portrait,landscape].");
        return false;
    }
    return true;
}

void OrientationCommand::RunSet()
{
    std::string commandOrientation = args["Orientation"].AsString();
    std::string currentOrientation = JsAppImpl::GetInstance().GetOrientation();
    if (commandOrientation != currentOrientation) {
        JsAppImpl::GetInstance().OrientationChanged(commandOrientation);
    }
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set Orientation run finished, Orientation is: %s", args["Orientation"].AsString().c_str());
}

ColorModeCommand::ColorModeCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool ColorModeCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("ColorMode") || !args["ColorMode"].IsString()) {
        ELOG("Invalid number of arguments!");
        return false;
    }

    if (args["ColorMode"].AsString() != "light" && args["ColorMode"].AsString() != "dark") {
        ELOG("ColorMode just support [light,dark]");
        return false;
    }
    return true;
}

void ColorModeCommand::RunSet()
{
    std::string commandColorMode = args["ColorMode"].AsString();
    std::string currentColorMode = JsAppImpl::GetInstance().GetColorMode();
    if (commandColorMode != currentColorMode) {
        JsAppImpl::GetInstance().SetArgsColorMode(args["ColorMode"].AsString());
        JsAppImpl::GetInstance().ColorModeChanged(commandColorMode);
    }
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set ColorMode run finished, ColorMode is: %s", args["ColorMode"].AsString().c_str());
}

FontSelectCommand::FontSelectCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool FontSelectCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("FontSelect") || !args["FontSelect"].IsBool()) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    return true;
}

void FontSelectCommand::RunSet()
{
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("FontSelect finished, currentSelect is: %s", args["FontSelect"].AsBool() ? "true" : "false");
}

MemoryRefreshCommand::MemoryRefreshCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool MemoryRefreshCommand::IsSetArgValid() const
{
    if (args.IsNull()) {
        ELOG("Invalid MemoryRefresh of arguments!");
        return false;
    }
    return true;
}

void MemoryRefreshCommand::RunSet()
{
    ILOG("MemoryRefreshCommand begin.");
    bool ret = JsAppImpl::GetInstance().MemoryRefresh(args.ToStyledString());
    SetCommandResult("result", JsonReader::CreateBool(ret));
    ILOG("MemoryRefresh finished.");
}

LoadDocumentCommand::LoadDocumentCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool LoadDocumentCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("url") || !args.IsMember("className") || !args.IsMember("previewParam") ||
        !args["url"].IsString() || !args["className"].IsString() || !args["previewParam"].IsObject()) {
        return false;
    }
    Json2::Value previewParam = args["previewParam"];
    if (!previewParam["width"].IsInt() || !previewParam["height"].IsInt() || !previewParam["dpi"].IsInt() ||
        !previewParam["locale"].IsString() || !previewParam["colorMode"].IsString() ||
        !previewParam["orientation"].IsString() || !previewParam["deviceType"].IsString()) {
        return false;
    }
    if (!IsIntValValid(previewParam) || !IsStrValVailid(previewParam)) {
        return false;
    }
    return true;
}

bool LoadDocumentCommand::IsIntValValid(const Json2::Value& previewParam) const
{
    int width = previewParam["width"].AsInt();
    int height = previewParam["height"].AsInt();
    int dpi = previewParam["dpi"].AsInt();
    if (width < minLoadDocWidth || width > maxLoadDocWidth || height < minLoadDocWidth ||
        height > maxLoadDocWidth || dpi < minDpi || dpi > maxDpi) {
        return false;
    }
    return true;
}

bool LoadDocumentCommand::IsStrValVailid(const Json2::Value& previewParam) const
{
    std::string locale = previewParam["locale"].AsString();
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
    if (previewParam["colorMode"].AsString() != "light" && previewParam["colorMode"].AsString() != "dark") {
        return false;
    }
    if (previewParam["orientation"].AsString() != "portrait" &&
        previewParam["orientation"].AsString() != "landscape") {
        return false;
    }
    if (std::find(LoadDocDevs.begin(), LoadDocDevs.end(), previewParam["deviceType"].AsString()) ==
        LoadDocDevs.end()) {
        return false;
    }
    return true;
}

void LoadDocumentCommand::RunSet()
{
    VirtualScreenImpl::GetInstance().SetLoadDocFlag(VirtualScreen::LoadDocType::START);
    ILOG("LoadDocumentCommand begin.");
    std::string pageUrl = args["url"].AsString();
    std::string className = args["className"].AsString();
    VirtualScreenImpl::GetInstance().InitFlushEmptyTime();
    JsAppImpl::GetInstance().LoadDocument(pageUrl, className, args["previewParam"]);
    VirtualScreenImpl::GetInstance().SetLoadDocFlag(VirtualScreen::LoadDocType::FINISHED);
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("LoadDocumentCommand finished.");
}

ReloadRuntimePageCommand::ReloadRuntimePageCommand(CommandType commandType,
                                                   const Json2::Value& arg,
                                                   const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool ReloadRuntimePageCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("ReloadRuntimePage") || !args["ReloadRuntimePage"].IsString()) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    return true;
}

void ReloadRuntimePageCommand::RunSet()
{
    std::string currentPage = args["ReloadRuntimePage"].AsString();
    JsAppImpl::GetInstance().ReloadRuntimePage(currentPage);
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("ReloadRuntimePage finished, currentPage is: %s", args["ReloadRuntimePage"].AsString().c_str());
}

CurrentRouterCommand::CurrentRouterCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void CurrentRouterCommand::RunGet()
{
    Json2::Value resultContent = JsonReader::CreateObject();
    std::string currentRouter = VirtualScreenImpl::GetInstance().GetCurrentRouter();
    resultContent.Add("CurrentRouter", currentRouter.c_str());
    SetResultToManager("args", resultContent, "CurrentJsRouter");
    ILOG("Get CurrentRouter run finished.");
}

LoadContentCommand::LoadContentCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void LoadContentCommand::RunGet()
{
    Json2::Value resultContent = JsonReader::CreateObject();
    std::string currentRouter = VirtualScreenImpl::GetInstance().GetAbilityCurrentRouter();
    resultContent.Add("AbilityCurrentRouter", currentRouter.c_str());
    SetResultToManager("args", resultContent, "AbilityCurrentJsRouter");
    ILOG("Get AbilityCurrentRouter run finished.");
}

LanguageCommand::LanguageCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool LanguageCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("Language") || !args["Language"].IsString()) {
        ELOG("Invalid number of arguments!");
        return false;
    }

    CommandParser& cmdParser = CommandParser::GetInstance();
    std::string deviceType = cmdParser.GetDeviceType();
    bool isLiteDevice = JsApp::IsLiteDevice(deviceType);
    if (isLiteDevice) {
        if (std::find(liteSupportedLanguages.begin(), liteSupportedLanguages.end(), args["Language"].AsString()) ==
            liteSupportedLanguages.end()) {
            ELOG("Language not support liteDevice : %s", args["Language"].AsString().c_str());
            return false;
        }
    } else {
        if (std::find(richSupportedLanguages.begin(), richSupportedLanguages.end(), args["Language"].AsString()) ==
            richSupportedLanguages.end()) {
            ELOG("Language not support richDevice : %s", args["Language"].AsString().c_str());
            return false;
        }
    }
    return true;
}

void LanguageCommand::RunGet()
{
    std::string language = SharedData<std::string>::GetData(SharedDataType::LANGUAGE);
    Json2::Value resultContent = JsonReader::CreateObject();
    resultContent.Add("Language", language.c_str());
    SetCommandResult("result", resultContent);
    ILOG("Get language run finished.");
}

void LanguageCommand::RunSet()
{
    std::string language(args["Language"].AsString());
    SharedData<std::string>::SetData(SharedDataType::LANGUAGE, language);
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set language run finished, language is: %s", language.c_str());
}

SupportedLanguagesCommand::SupportedLanguagesCommand(CommandType commandType,
                                                     const Json2::Value& arg,
                                                     const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void SupportedLanguagesCommand::RunGet()
{
    Json2::Value resultContent = JsonReader::CreateObject();
    Json2::Value languageList = JsonReader::CreateArray();
    std::string deviceType = CommandParser::GetInstance().GetDeviceType();
    bool isLiteDevice = JsApp::IsLiteDevice(deviceType);
    if (!deviceType.empty() && !isLiteDevice) {
        for (auto iter = richSupportedLanguages.begin(); iter != richSupportedLanguages.end(); iter++) {
            languageList.Add((*iter).c_str());
        }
    } else {
        for (auto iter = liteSupportedLanguages.begin(); iter != liteSupportedLanguages.end(); iter++) {
            languageList.Add((*iter).c_str());
        }
    }
    resultContent.Add("SupportedLanguages", languageList);
    SetCommandResult("result", resultContent);
    ILOG("Get supportedLanguages run finished.");
}

LocationCommand::LocationCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool LocationCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("latitude") || !args.IsMember("longitude")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    std::string latitude(args["latitude"].AsString());
    std::string longitude(args["longitude"].AsString());
    std::regex isDob("^([\\-]*[0-9]{1,}[\\.][0-9]*)$");
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
    Json2::Value resultContent = JsonReader::CreateObject();
    resultContent.Add("latitude", latitude);
    resultContent.Add("longitude", longitude);
    SetCommandResult("result", resultContent);
    ILOG("Get location run finished");
}

void LocationCommand::RunSet()
{
    std::string latitude(args["latitude"].AsString());
    std::string longitude(args["longitude"].AsString());
    SharedData<double>::SetData(SharedDataType::LONGITUDE, atof(longitude.data()));
    SharedData<double>::SetData(SharedDataType::LATITUDE, atof(latitude.data()));
    Json2::Value resultContent = JsonReader::CreateBool(true);
    SetCommandResult("result", resultContent);
    ILOG("Set location run finished, latitude: %s,longitude: %s", latitude.c_str(), longitude.c_str());
}

DistributedCommunicationsCommand::DistributedCommunicationsCommand(CommandType commandType,
                                                                   const Json2::Value& arg,
                                                                   const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void DistributedCommunicationsCommand::RunAction()
{
    MessageInfo info;
    info.deviceID = args["DeviceId"].AsString();
    info.bundleName = args["bundleName"].AsString();
    info.abilityName = args["abilityName"].AsString();
    info.message = args["message"].AsString();
    VirtualMessageImpl::GetInstance().SendVirtualMessage(info);
    Json2::Value resultContent = JsonReader::CreateBool(true);
    SetCommandResult("result", resultContent);
    ILOG("Send distributedCommunications run finished");
}

bool DistributedCommunicationsCommand::IsActionArgValid() const
{
    if (args.IsNull() || !args.IsMember("DeviceId") || !args.IsMember("bundleName") || !args.IsMember("abilityName") ||
        !args.IsMember("message")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (args["DeviceId"].AsString().empty() || args["bundleName"].AsString().empty() ||
        args["abilityName"].AsString().empty() || args["message"].AsString().empty()) {
        ELOG("Invalid arguments!");
        return false;
    }
    return true;
}

std::vector<char> DistributedCommunicationsCommand::StringToCharVector(std::string str) const
{
    std::vector<char> vec(str.begin(), str.end());
    vec.push_back('\0');
    return vec;
}

KeepScreenOnStateCommand::KeepScreenOnStateCommand(CommandType commandType,
                                                   const Json2::Value& arg,
                                                   const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void KeepScreenOnStateCommand::RunGet()
{
    Json2::Value result = JsonReader::CreateObject();
    result.Add("KeepScreenOnState", SharedData<bool>::GetData(SharedDataType::KEEP_SCREEN_ON));
    SetCommandResult("result", result);
    ILOG("Get keepScreenOnState run finished");
}

void KeepScreenOnStateCommand::RunSet()
{
    SharedData<bool>::SetData(SharedDataType::KEEP_SCREEN_ON, args["KeepScreenOnState"].AsBool());
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set keepScreenOnState run finished, the value is: %s",
        args["KeepScreenOnState"].AsBool() ? "true" : "false");
}

bool KeepScreenOnStateCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("KeepScreenOnState")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!args["KeepScreenOnState"].IsBool()) {
        ELOG("arg KeepScreenOnState id not bool");
        return false;
    }
    return true;
}

WearingStateCommand::WearingStateCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void WearingStateCommand::RunGet()
{
    Json2::Value result = JsonReader::CreateObject();
    result.Add("WearingState", SharedData<bool>::GetData(SharedDataType::WEARING_STATE));
    SetCommandResult("result", result);
    ILOG("Get wearingState run finished");
}

void WearingStateCommand::RunSet()
{
    SharedData<bool>::SetData(SharedDataType::WEARING_STATE, args["WearingState"].AsBool());
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set wearingState run finished, the value is: %s", args["WearingState"].AsBool() ? "true" : "false");
}

bool WearingStateCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("WearingState")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!args["WearingState"].IsBool()) {
        ILOG("arg WearingState is not bool");
        return false;
    }
    return true;
}

BrightnessModeCommand::BrightnessModeCommand(CommandType commandType, const Json2::Value& arg,
    const LocalSocket& socket) : CommandLine(commandType, arg, socket)
{
}

void BrightnessModeCommand::RunGet()
{
    Json2::Value result = JsonReader::CreateObject();
    result.Add("BrightnessMode", SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_MODE));
    SetCommandResult("result", result);
    ILOG("Get brightnessMode run finished");
}

void BrightnessModeCommand::RunSet()
{
    SharedData<uint8_t>::SetData(SharedDataType::BRIGHTNESS_MODE,
                                 static_cast<uint8_t>(args["BrightnessMode"].AsInt()));
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set brightnessMode run finished, the value is: %d", args["BrightnessMode"].AsInt());
}

bool BrightnessModeCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("BrightnessMode")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!args["BrightnessMode"].IsInt()) {
        ELOG("BrightnessMode is not int");
        return false;
    }
    uint8_t temp = static_cast<uint8_t>(args["BrightnessMode"].AsInt());
    if (!SharedData<uint8_t>::IsValid(SharedDataType::BRIGHTNESS_MODE, temp)) {
        ELOG("BrightnessModeCommand invalid value: %d", temp);
        return false;
    }
    return true;
}

ChargeModeCommand::ChargeModeCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void ChargeModeCommand::RunGet()
{
    Json2::Value result = JsonReader::CreateObject();
    result.Add("ChargeMode", SharedData<uint8_t>::GetData(SharedDataType::BATTERY_STATUS));
    SetCommandResult("result", result);
    ILOG("Get chargeMode run finished");
}

void ChargeModeCommand::RunSet()
{
    SharedData<uint8_t>::SetData(SharedDataType::BATTERY_STATUS,
                                 static_cast<uint8_t>(args["ChargeMode"].AsInt()));
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set chargeMode run finished, the value is: %d", args["ChargeMode"].AsInt());
}

bool ChargeModeCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("ChargeMode")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!args["ChargeMode"].IsInt()) {
        ELOG("ChargeMode is not int");
        return false;
    }
    uint8_t temp = static_cast<uint8_t>(args["ChargeMode"].AsInt());
    if (!SharedData<uint8_t>::IsValid(SharedDataType::BATTERY_STATUS, temp)) {
        ELOG("ChargeModeCommand invalid value: %d", temp);
        return false;
    }
    return true;
}

BrightnessCommand::BrightnessCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void BrightnessCommand::RunGet()
{
    Json2::Value result = JsonReader::CreateObject();
    result.Add("Brightness", SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_VALUE));
    SetCommandResult("result", result);
    ILOG("Get brightness run finished");
}

void BrightnessCommand::RunSet()
{
    SharedData<uint8_t>::SetData(SharedDataType::BRIGHTNESS_VALUE,
                                 static_cast<uint8_t>(args["Brightness"].AsInt()));
    Json2::Value result = JsonReader::CreateBool(true);
    SetCommandResult("result", result);
    ILOG("Set brightness run finished, the value is: %d", args["Brightness"].AsInt());
}

bool BrightnessCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("Brightness")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!args["Brightness"].IsInt()) {
        ELOG("Brightness is not int");
        return false;
    }
    uint8_t temp = static_cast<uint8_t>(args["Brightness"].AsInt());
    if (!SharedData<uint8_t>::IsValid(SharedDataType::BRIGHTNESS_VALUE, temp)) {
        ELOG("BrightnessCommand invalid value: ", temp);
        return false;
    }
    return true;
}

HeartRateCommand::HeartRateCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void HeartRateCommand::RunGet()
{
    Json2::Value result = JsonReader::CreateObject();
    result.Add("HeartRate", SharedData<uint8_t>::GetData(SharedDataType::HEARTBEAT_VALUE));
    SetCommandResult("result", result);
    ILOG("Get heartRate run finished");
}

void HeartRateCommand::RunSet()
{
    SharedData<uint8_t>::SetData(SharedDataType::HEARTBEAT_VALUE,
                                 static_cast<uint8_t>(args["HeartRate"].AsInt()));
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set heartRate run finished, the value is: %d", args["HeartRate"].AsInt());
}

bool HeartRateCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("HeartRate")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!args["HeartRate"].IsInt()) {
        ELOG("HeartRate is not int");
        return false;
    }
    if (args["HeartRate"].AsInt() > UINT8_MAX) {
        ELOG("Invalid arguments!");
        return false;
    }
    uint8_t temp = static_cast<uint8_t>(args["HeartRate"].AsInt());
    if (!SharedData<uint8_t>::IsValid(SharedDataType::HEARTBEAT_VALUE, temp)) {
        ELOG("HeartRateCommand invalid value: %d", temp);
        return false;
    }
    return true;
}

StepCountCommand::StepCountCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void StepCountCommand::RunGet()
{
    Json2::Value result = JsonReader::CreateObject();
    result.Add("StepCount", SharedData<uint32_t>::GetData(SharedDataType::SUMSTEP_VALUE));
    SetCommandResult("result", result);
    ILOG("Get stepCount run finished");
}

void StepCountCommand::RunSet()
{
    SharedData<uint32_t>::SetData(SharedDataType::SUMSTEP_VALUE,
                                  static_cast<uint32_t>(args["StepCount"].AsInt()));
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set stepCount run finished, the value is: %d", args["StepCount"].AsInt());
}

bool StepCountCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("StepCount")) {
        ELOG("Invalid number of arguments!");
        return false;
    }
    if (!args["StepCount"].IsInt()) {
        ELOG("StepCount is not int");
        return false;
    }

    uint32_t temp = args["StepCount"].AsUInt();
    if (!SharedData<uint32_t>::IsValid(SharedDataType::SUMSTEP_VALUE, temp)) {
        ELOG("StepCountCommand invalid value: %d", temp);
        return false;
    }
    return true;
}

InspectorJSONTree::InspectorJSONTree(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
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
    SetCommandResult("result", JsonReader::CreateString(str));
    ILOG("SendJsonTree end!");
}

InspectorDefault::InspectorDefault(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void InspectorDefault::RunAction()
{
    ILOG("GetDefaultJsonTree run!");
    std::string str = JsAppImpl::GetInstance().GetDefaultJSONTree();
    SetCommandResult("result", JsonReader::CreateString(str));
    ILOG("SendDefaultJsonTree end!");
}

ExitCommand::ExitCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void ExitCommand::RunAction()
{
    ILOG("ExitCommand run.");
    SetCommandResult("result", JsonReader::CreateBool(true));
    SendResult();
    Interrupter::Interrupt();
    ILOG("Ready to exit");
}

DeviceTypeCommand::DeviceTypeCommand(CommandLine::CommandType commandType,
                                     const Json2::Value& arg,
                                     const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void DeviceTypeCommand::RunSet() {}

ResolutionCommand::ResolutionCommand(CommandLine::CommandType commandType,
                                     const Json2::Value& arg,
                                     const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void ResolutionCommand::RunSet() {}

BackClickedCommand::BackClickedCommand(CommandLine::CommandType commandType,
                                       const Json2::Value& arg,
                                       const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void BackClickedCommand::RunAction()
{
    MouseInputImpl::GetInstance().DispatchOsBackEvent();
    ILOG("BackClickCommand run");
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("BackClickCommand end");
}

RestartCommand::RestartCommand(CommandLine::CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void RestartCommand::RunAction()
{
    ILOG("RestartCommand start");
    JsAppImpl::GetInstance().Restart();
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("RestartCommand end");
}

FastPreviewMsgCommand::FastPreviewMsgCommand(CommandType commandType, const Json2::Value& arg,
    const LocalSocket& socket) : CommandLine(commandType, arg, socket)
{
}

void FastPreviewMsgCommand::RunGet()
{
    Json2::Value resultContent = JsonReader::CreateObject();
    std::string fastPreviewMsg = VirtualScreenImpl::GetInstance().GetFastPreviewMsg();
    resultContent.Add("FastPreviewMsg", fastPreviewMsg.c_str());
    SetResultToManager("args", resultContent, "MemoryRefresh");
    ILOG("Get FastPreviewMsgCommand run finished.");
}

DropFrameCommand::DropFrameCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool DropFrameCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("frequency") || !args["frequency"].IsInt()) {
        ELOG("Invalid DropFrame of arguments!");
        return false;
    }
    if (args["frequency"].AsInt() < 0) {
        ELOG("DropFrame param frequency must greater than or equal to 0");
        return false;
    }
    return true;
}

void DropFrameCommand::RunSet()
{
    ILOG("Set DropFrame frequency start.");
    int frequency = args["frequency"].AsInt();
    VirtualScreenImpl::GetInstance().SetDropFrameFrequency(frequency);
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set DropFrame frequency: %dms.", frequency);
}

bool KeyPressCommand::IsActionArgValid() const
{
    if (args.IsNull() || !args.IsMember("isInputMethod") || !args["isInputMethod"].IsBool()) {
        ELOG("Param isInputMethod's value is invalid.");
        return false;
    }
    bool isInputMethod = args["isInputMethod"].AsBool();
    if (isInputMethod) {
        return IsImeArgsValid();
    } else {
        return IsKeyArgsValid();
    }
}

bool KeyPressCommand::IsImeArgsValid() const
{
    if (!args.IsMember("codePoint") || !args["codePoint"].IsInt()) {
        ELOG("Param codePoint's value is invalid.");
        return false;
    }
    return true;
}

bool KeyPressCommand::IsKeyArgsValid() const
{
    if (!args.IsMember("keyCode") || !args["keyCode"].IsInt() || !args["keyAction"].IsInt() ||
        !args.IsMember("keyAction") || !args["keyAction"].IsInt() ||
        !args.IsMember("pressedCodes") || !args["pressedCodes"].IsArray() ||
        args["pressedCodes"].GetArraySize() < 1 || (args.IsMember("keyString") && !args["keyString"].IsString())) {
        ELOG("Param keyEvent's value is invalid.");
        return false;
    }
    if (!args.IsMember("keyString")) {
        ILOG("Param keyString is lost, it will be empty string.");
    }
    if (args["keyAction"].AsInt() < minActionVal || args["keyAction"].AsInt() > maxActionVal) {
        ELOG("Param keyAction's value is invalid,value range %d-%d.", minActionVal, maxActionVal);
        return false;
    }
    int keyCode = args["keyCode"].AsInt();
    if (keyCode > maxKeyVal || keyCode < minKeyVal) {
        ELOG("Param pressedCode value is invalid,value range %d-%d.", minKeyVal, maxKeyVal);
        return false;
    }
    Json2::Value arrayNum = args["pressedCodes"];
    for (unsigned int i = 0; i < arrayNum.GetArraySize(); i++) {
        if (!arrayNum.GetArrayItem(i).IsInt()) {
            ELOG("Param pressedCodes's value is invalid.");
            return false;
        }
        int pressedCode = arrayNum.GetArrayItem(i).AsInt();
        if (pressedCode > maxKeyVal || pressedCode < minKeyVal) {
            ELOG("Param pressedCode value is invalid,value range %d-%d.", minKeyVal, maxKeyVal);
            return false;
        }
    }
    return true;
}

KeyPressCommand::KeyPressCommand(CommandType commandType, const Json2::Value& arg,
                                 const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void KeyPressCommand::RunAction()
{
    if (CommandParser::GetInstance().GetScreenMode() == CommandParser::ScreenMode::STATIC) {
        return;
    }
    bool isInputMethod = args["isInputMethod"].AsBool();
    if (isInputMethod) {
        VirtualScreen::inputMethodCountPerMinute++;
        unsigned int codePoint = args["codePoint"].AsInt();
        KeyInputImpl::GetInstance().SetCodePoint(codePoint);
        KeyInputImpl::GetInstance().DispatchOsInputMethodEvent();
    } else {
        VirtualScreen::inputKeyCountPerMinute++;
        int32_t keyCode = args["keyCode"].AsInt();
        int32_t keyAction = args["keyAction"].AsInt();
        Json2::Value pressedCodes = args["pressedCodes"];
        std::vector<int32_t> pressedCodesVec;
        for (unsigned int i = 0; i < pressedCodes.GetArraySize(); i++) {
            pressedCodesVec.push_back(pressedCodes.GetArrayItem(i).AsInt());
        }
        std::string keyString = "";
        if (args.IsMember("keyString") && args["keyString"].IsString()) {
            keyString = args["keyString"].AsString();
        }
        KeyInputImpl::GetInstance().SetKeyEvent(keyCode, keyAction, pressedCodesVec, keyString);
        KeyInputImpl::GetInstance().DispatchOsKeyEvent();
    }
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("KeyPressCommand run finished.");
}

bool PointEventCommand::IsActionArgValid() const
{
    return IsArgsExist() && IsArgsValid();
}

bool PointEventCommand::IsArgsExist() const
{
    if (args.IsNull() || !args.IsMember("x") || !args.IsMember("y") ||
        !args["x"].IsInt() || !args["y"].IsInt()) {
        return false;
    }
    if (!args.IsMember("button") || !args.IsMember("action") ||
        !args["button"].IsInt() || !args["action"].IsInt()) {
        return false;
    }
    if (!args.IsMember("sourceType") || !args.IsMember("sourceTool") ||
        !args["sourceType"].IsInt() || !args["sourceTool"].IsInt()) {
        return false;
    }
    if (!args.IsMember("axisValues") || !args["axisValues"].IsArray()) {
        return false;
    }
    return true;
}

bool PointEventCommand::IsArgsValid() const
{
    int32_t pointX = args["x"].AsInt();
    int32_t pointY = args["y"].AsInt();
    int32_t button = args["button"].AsInt();
    int32_t action = args["action"].AsInt();
    int32_t sourceType = args["sourceType"].AsInt();
    int32_t sourceTool = args["sourceTool"].AsInt();
    if (pointX < 0 || pointX > VirtualScreenImpl::GetInstance().GetCurrentWidth()) {
        ELOG("X coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetCurrentWidth());
        return false;
    }
    if (pointY < 0 || pointY > VirtualScreenImpl::GetInstance().GetCurrentHeight()) {
        ELOG("Y coordinate range %d ~ %d", 0, VirtualScreenImpl::GetInstance().GetCurrentHeight());
        return false;
    }
    if (button < -1 || action < 0 || sourceType < 0 || sourceTool < 0) {
        ELOG("action,sourceType,sourceTool must >= 0, button must >= -1");
        return false;
    }
    Json2::Value axisArrayNum = args["axisValues"];
    for (unsigned int i = 0; i < axisArrayNum.GetArraySize(); i++) {
        if (!axisArrayNum.GetArrayItem(i).IsDouble()) {
            ELOG("Param axisValues's value is invalid.");
            return false;
        }
    }
    return true;
}

PointEventCommand::PointEventCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void PointEventCommand::RunAction()
{
    int type = 9;
    EventParams param;
    if (args.IsMember("pressedButtons") && args["pressedButtons"].IsArray()) {
        Json2::Value pressedCodes = args["pressedButtons"];
        for (unsigned int i = 0; i < pressedCodes.GetArraySize(); i++) {
            Json2::Value val = pressedCodes.GetArrayItem(i);
            if (!val.IsInt() || val.AsInt() < -1) {
                continue;
            }
            param.pressedBtnsVec.insert(val.AsInt());
        }
    }
    std::vector<double> axisVec; // 13 is array size
    Json2::Value axisCodes = args["axisValues"];
    for (unsigned int i = 0; i < axisCodes.GetArraySize(); i++) {
        param.axisVec.push_back(axisCodes.GetArrayItem(i).AsDouble());
    }
    param.x = args["x"].AsDouble();
    param.y = args["y"].AsDouble();
    param.type = type;
    param.button = args["button"].AsInt();
    param.action = args["action"].AsInt();
    param.sourceType = args["sourceType"].AsInt();
    param.sourceTool = args["sourceTool"].AsInt();
    param.name = "PointEvent";
    SetEventParams(param);
    SetCommandResult("result", JsonReader::CreateBool(true));
}

FoldStatusCommand::FoldStatusCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

bool FoldStatusCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("FoldStatus") || !args["FoldStatus"].IsString()) {
        ELOG("Invalid FoldStatus of arguments!");
        return false;
    }
    if (!args.IsMember("width") || !args["width"].IsInt() ||
        !args.IsMember("height") || !args["height"].IsInt()) {
        ELOG("Invalid width and height of arguments!");
        return false;
    }
    if (args["width"].AsInt() < minWidth || args["width"].AsInt() > maxWidth ||
        args["height"].AsInt() < minWidth || args["height"].AsInt() > maxWidth) {
        ELOG("width or height is out of range %d-%d", minWidth, maxWidth);
        return false;
    }
    if (args["FoldStatus"].AsString() == "fold" || args["FoldStatus"].AsString() == "unfold" ||
        args["FoldStatus"].AsString() == "unknown" || args["FoldStatus"].AsString() == "half_fold") {
        return true;
    }
    ELOG("FoldStatus param must be \"fold\" or \"unfold\" or \"unknown\" or \"half_fold\"");
    return false;
}

void FoldStatusCommand::RunSet()
{
    std::string commandStatus = args["FoldStatus"].AsString();
    int32_t width = args["width"].AsInt();
    int32_t height = args["height"].AsInt();
    std::string currentStatus = VirtualScreenImpl::GetInstance().GetFoldStatus();
    if (commandStatus != currentStatus) {
        JsAppImpl::GetInstance().FoldStatusChanged(commandStatus, width, height);
    }
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set FoldStatus run finished, FoldStatus is: %s", args["FoldStatus"].AsString().c_str());
}

AvoidAreaCommand::AvoidAreaCommand(CommandType commandType, const Json2::Value& arg,
    const LocalSocket& socket) : CommandLine(commandType, arg, socket)
{
}

bool AvoidAreaCommand::IsSetArgValid() const
{
    if (args.IsNull() || !args.IsMember("topRect") || !args.IsMember("bottomRect") ||
        !args.IsMember("leftRect") || !args.IsMember("rightRect")) {
        ELOG("AvoidAreaCommand missing arguments!");
        return false;
    }
    if (!args["topRect"].IsObject() || !args["bottomRect"].IsObject() || !args["leftRect"].IsObject()
        || !args["rightRect"].IsObject()) {
        ELOG("Invalid values of arguments!");
        return false;
    }
    Json2::Value topRect = args.GetValue("topRect");
    Json2::Value bottomRect = args.GetValue("bottomRect");
    Json2::Value leftRect = args.GetValue("leftRect");
    Json2::Value rightRect = args.GetValue("rightRect");
    if (!IsObjectValid(topRect) || !IsObjectValid(bottomRect) ||
        !IsObjectValid(leftRect) || !IsObjectValid(rightRect)) {
        ELOG("Invalid values of arguments!");
        return false;
    }
    return true;
}

bool AvoidAreaCommand::IsObjectValid(const Json2::Value& val) const
{
    if (val.IsNull() || !val.IsMember("posX") || !val["posY"].IsInt() ||
        !val.IsMember("width") || !val.IsMember("height")) {
        ELOG("AvoidAreaCommand missing arguments!");
        return false;
    }
    if (!val["posX"].IsInt() || !val["posY"].IsInt() || !val["width"].IsUInt()
        || !val["height"].IsUInt()) {
        ELOG("Invalid values of arguments!");
        return false;
    }
    if (val["posX"].AsInt() < 0 || val["posY"].AsInt() < 0 ||
        val["width"].AsUInt() < 0 || val["height"].AsUInt() < 0) {
        ELOG("Invalid values of arguments!");
        return false;
    }
    return true;
}

void AvoidAreaCommand::RunSet()
{
    Json2::Value topRectObj = args.GetValue("topRect");
    AvoidRect topRect = AvoidRect(topRectObj["posX"].AsInt(), topRectObj["posY"].AsInt(),
        topRectObj["width"].AsUInt(), topRectObj["height"].AsUInt());
    Json2::Value bottomRectObj = args.GetValue("bottomRect");
    AvoidRect bottomRect = AvoidRect(bottomRectObj["posX"].AsInt(), bottomRectObj["posY"].AsInt(),
        bottomRectObj["width"].AsUInt(), bottomRectObj["height"].AsUInt());
    Json2::Value leftRectObj = args.GetValue("leftRect");
    AvoidRect leftRect = AvoidRect(leftRectObj["posX"].AsInt(), leftRectObj["posY"].AsInt(),
        leftRectObj["width"].AsUInt(), leftRectObj["height"].AsUInt());
    Json2::Value rightRectObj = args.GetValue("rightRect");
    AvoidRect rightRect = AvoidRect(rightRectObj["posX"].AsInt(), rightRectObj["posY"].AsInt(),
        rightRectObj["width"].AsUInt(), rightRectObj["height"].AsUInt());
    JsAppImpl::GetInstance().SetAvoidArea(AvoidAreas(topRect, leftRect, rightRect, bottomRect));
    SetCommandResult("result", JsonReader::CreateBool(true));
    ILOG("Set AvoidArea run finished");
}

AvoidAreaChangedCommand::AvoidAreaChangedCommand(CommandType commandType, const Json2::Value& arg,
    const LocalSocket& socket) : CommandLine(commandType, arg, socket)
{
}

void AvoidAreaChangedCommand::RunGet()
{
    SetResultToManager("args", args, "AvoidAreaChanged");
    ILOG("Get AvoidAreaChangedCommand run finished.");
}

static std::string GetTimeString()
{
    auto now = std::chrono::system_clock::now();
    auto cNow = std::chrono::system_clock::to_time_t(now);
    tm* tmNow = localtime(&cNow);

    std::stringstream timeString;
    timeString << std::put_time(tmNow, "%Y-%m-%d_%H-%M-%S");
    return timeString.str();
}

ScreenShotCommand::ScreenShotCommand(CommandType commandType, const Json2::Value& arg, const LocalSocket& socket)
    : CommandLine(commandType, arg, socket)
{
}

void ScreenShotCommand::RunAction()
{
    VirtualScreenImpl::GetInstance().MakeScreenShot("screen_" + GetTimeString());
    SetCommandResult("result", JsonReader::CreateBool(true));
}

StartVideoRecordCommand::StartVideoRecordCommand(CommandType commandType, const Json2::Value& arg,
    const LocalSocket& socket) : CommandLine(commandType, arg, socket)
{
}

void StartVideoRecordCommand::RunAction()
{
    SetCommandResult("result", JsonReader::CreateBool(true));
}

StopVideoRecordCommand::StopVideoRecordCommand(CommandType commandType, const Json2::Value& arg,
    const LocalSocket& socket) : CommandLine(commandType, arg, socket)
{
}

void StopVideoRecordCommand::RunAction()
{
    SetCommandResult("result", JsonReader::CreateBool(true));
}

