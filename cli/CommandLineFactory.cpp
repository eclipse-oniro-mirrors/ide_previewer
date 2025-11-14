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

#include "CommandLineFactory.h"

#include "CommandLineInterface.h"
#include "CommandParser.h"
#include "JsApp.h"
#include "PreviewerEngineLog.h"
#include "TraceTool.h"

CommandLineFactory::CommandTypeMap CommandLineFactory::typeMap = CommandLineFactory::CommandTypeMap();
CommandLineFactory::CommandLineFactory() {}

void CommandLineFactory::InitCommandMap()
{
    CommandParser& cmdParser = CommandParser::GetInstance();
    std::string deviceType = cmdParser.GetDeviceType();
    bool isLiteDevice = JsApp::IsLiteDevice(deviceType);
    if (!isLiteDevice) {
        typeMap["BackClicked"] = &CommandLineFactory::CreateObject<BackClickedCommand>;
        typeMap["inspector"] = &CommandLineFactory::CreateObject<InspectorJSONTree>;
        typeMap["inspectorDefault"] = &CommandLineFactory::CreateObject<InspectorDefault>;
        typeMap["ColorMode"] = &CommandLineFactory::CreateObject<ColorModeCommand>;
        typeMap["Orientation"] = &CommandLineFactory::CreateObject<OrientationCommand>;
        typeMap["ResolutionSwitch"] = &CommandLineFactory::CreateObject<ResolutionSwitchCommand>;
        typeMap["CurrentRouter"] = &CommandLineFactory::CreateObject<CurrentRouterCommand>;
        typeMap["ReloadRuntimePage"] = &CommandLineFactory::CreateObject<ReloadRuntimePageCommand>;
        typeMap["FontSelect"] = &CommandLineFactory::CreateObject<FontSelectCommand>;
        typeMap["MemoryRefresh"] = &CommandLineFactory::CreateObject<MemoryRefreshCommand>;
        typeMap["LoadDocument"] = &CommandLineFactory::CreateObject<LoadDocumentCommand>;
        typeMap["FastPreviewMsg"] = &CommandLineFactory::CreateObject<FastPreviewMsgCommand>;
        typeMap["DropFrame"] = &CommandLineFactory::CreateObject<DropFrameCommand>;
        typeMap["KeyPress"] = &CommandLineFactory::CreateObject<KeyPressCommand>;
        typeMap["LoadContent"] = &CommandLineFactory::CreateObject<LoadContentCommand>;
        typeMap["FoldStatus"] = &CommandLineFactory::CreateObject<FoldStatusCommand>;
        typeMap["AvoidArea"] = &CommandLineFactory::CreateObject<AvoidAreaCommand>;
        typeMap["AvoidAreaChanged"] = &CommandLineFactory::CreateObject<AvoidAreaChangedCommand>;
    } else {
        typeMap["Power"] = &CommandLineFactory::CreateObject<PowerCommand>;
        typeMap["Volume"] = &CommandLineFactory::CreateObject<VolumeCommand>;
        typeMap["Barometer"] = &CommandLineFactory::CreateObject<BarometerCommand>;
        typeMap["Location"] = &CommandLineFactory::CreateObject<LocationCommand>;
        typeMap["KeepScreenOnState"] = &CommandLineFactory::CreateObject<KeepScreenOnStateCommand>;
        typeMap["WearingState"] = &CommandLineFactory::CreateObject<WearingStateCommand>;
        typeMap["BrightnessMode"] = &CommandLineFactory::CreateObject<BrightnessModeCommand>;
        typeMap["ChargeMode"] = &CommandLineFactory::CreateObject<ChargeModeCommand>;
        typeMap["Brightness"] = &CommandLineFactory::CreateObject<BrightnessCommand>;
        typeMap["HeartRate"] = &CommandLineFactory::CreateObject<HeartRateCommand>;
        typeMap["StepCount"] = &CommandLineFactory::CreateObject<StepCountCommand>;
        typeMap["DistributedCommunications"] = &CommandLineFactory::CreateObject<DistributedCommunicationsCommand>;
        typeMap["CrownRotate"] = &CommandLineFactory::CreateObject<MouseWheelCommand>;
    }
    typeMap["MousePress"] = &CommandLineFactory::CreateObject<TouchPressCommand>;
    typeMap["MouseRelease"] = &CommandLineFactory::CreateObject<TouchReleaseCommand>;
    typeMap["MouseMove"] = &CommandLineFactory::CreateObject<TouchMoveCommand>;
    typeMap["Language"] = &CommandLineFactory::CreateObject<LanguageCommand>;
    typeMap["SupportedLanguages"] = &CommandLineFactory::CreateObject<SupportedLanguagesCommand>;
    typeMap["exit"] = &CommandLineFactory::CreateObject<ExitCommand>;
    typeMap["Resolution"] = &CommandLineFactory::CreateObject<ResolutionCommand>;
    typeMap["DeviceType"] = &CommandLineFactory::CreateObject<DeviceTypeCommand>;
    typeMap["PointEvent"] = &CommandLineFactory::CreateObject<PointEventCommand>;
}

std::unique_ptr<CommandLine> CommandLineFactory::CreateCommandLine(std::string command,
    CommandLine::CommandType type, const Json2::Value& val, const LocalSocket& socket)
{
    if (typeMap.find(command) == typeMap.end()) {
        Json2::Value commandResult = JsonReader::CreateObject();
        commandResult.Add("version", CommandLineInterface::COMMAND_VERSION.c_str());
        commandResult.Add("command", command.c_str());
        commandResult.Add("result", "Unsupported command");
        socket << commandResult.ToStyledString();
        ELOG("Unsupported command");
        TraceTool::GetInstance().HandleTrace("Mismatched SDK version");
        return nullptr;
    }
    if (typeMap[command] == nullptr) {
        ELOG("CommandLineFactory::CreateCommandLine:typeMap is null");
    }
    ILOG("Create Command: %s", command.c_str());
    std::unique_ptr<CommandLine> cmdLine = typeMap[command](type, val, socket);
    if (cmdLine == nullptr) {
        ELOG("CommandLineFactory::CreateCommandLine:cmdLine is null");
        return nullptr;
    }
    cmdLine->SetCommandName(command);
    return cmdLine;
}

template <typename T>
std::unique_ptr<CommandLine> CommandLineFactory::CreateObject(CommandLine::CommandType type,
    const Json2::Value& args, const LocalSocket& socket)
{
    return std::make_unique<T>(type, args, socket);
}
