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

#include <chrono>
#include <thread>
#include <vector>

#include "CommandLineInterface.h"
#include "CommandParser.h"
#include "CppTimer.h"
#include "CppTimerManager.h"
#include "CrashHandler.h"
#include "Interrupter.h"
#include "JsAppImpl.h"
#include "PreviewerEngineLog.h"
#include "SharedData.h"
#include "TraceTool.h"
#include "VirtualScreenImpl.h"
#include "json.h"

using namespace std;
static const int START_PARAM_INVALID_CODE = 11;
static const int NOTIFY_INTERVAL_TIME = 1000; // Unit millisecond

static void InitDeviceOrientation()
{
    CommandParser& parser = CommandParser::GetInstance();
    if (parser.GetCompressionResolutionWidth() <= parser.GetCompressionResolutionHeight()) {
        ILOG("InitDeviceOrientation is portrait.");
        JsAppImpl::GetInstance().SetDeviceOrentation("portrait");
    } else {
        ILOG("InitDeviceOrientation is landscape.");
        JsAppImpl::GetInstance().SetDeviceOrentation("landscape");
    }
}

static void InitJsApp()
{
    CommandParser& parser = CommandParser::GetInstance();
    // Initialize Image Pipeline Name
    if (parser.IsSet("s")) {
        JsAppImpl::GetInstance().SetPipeName(parser.Value("s"));
    }

    if (parser.IsSet("lws")) {
        JsAppImpl::GetInstance().SetPipePort(parser.Value("lws"));
    }

    JsAppImpl::GetInstance().SetJsAppPath(parser.Value("j"));

    if (parser.IsSet("url")) {
        JsAppImpl::GetInstance().SetUrlPath(parser.Value("url"));
    }

    if (parser.IsSet("cm")) {
        JsAppImpl::GetInstance().SetArgsColorMode(parser.Value("cm"));
    }

    if (parser.IsSet("av")) {
        JsAppImpl::GetInstance().SetArgsAceVersion(parser.Value("av"));
    }

    if (parser.IsSet("sd")) {
        JsAppImpl::GetInstance().SetScreenDensity(parser.Value("sd"));
    }

    if (parser.IsSet("cc")) {
        JsAppImpl::GetInstance().SetConfigChanges(parser.Value("cc"));
    }

    InitDeviceOrientation();

    if (parser.IsSet("d")) {
        JsAppImpl::GetInstance().SetIsDebug(true);
        if (parser.IsSet("p")) {
            JsAppImpl::GetInstance().SetDebugServerPort(static_cast<uint16_t>(atoi(parser.Value("p").c_str())));
        }
    }

    JsAppImpl::GetInstance().Start();
}

static void NotifyInspectorChanged()
{
    if (!VirtualScreenImpl::GetInstance().isFrameUpdated) {
        return;
    }
    VirtualScreenImpl::GetInstance().isFrameUpdated = false;

    static std::string jsonTreeLast = "";
    std::string jsonTree = JsAppImpl::GetInstance().GetJSONTree();
    if (jsonTree == jsonTreeLast) {
        return;
    }

    jsonTreeLast = jsonTree;
    Json::Value commandResult;
    commandResult["version"] = CommandLineInterface::COMMAND_VERSION;
    commandResult["command"] = "inspector";
    commandResult["result"] = jsonTree;
    CommandLineInterface::GetInstance().SendJsonData(commandResult);
    ILOG("Send inspector json tree.");
}

static void ProcessCommand()
{
    if (!CommandParser::GetInstance().IsSet("d")) {
        static CppTimer inspectorNotifytimer(NotifyInspectorChanged);
        inspectorNotifytimer.Start(NOTIFY_INTERVAL_TIME); // Notify per second
        CppTimerManager::GetTimerManager().AddCppTimer(inspectorNotifytimer);
    }

    VirtualScreenImpl::GetInstance().InitFrameCountTimer();
    while (!Interrupter::IsInterrupt()) {
        CommandLineInterface::GetInstance().ProcessCommand();
        CppTimerManager::GetTimerManager().RunTimerTick();
        this_thread::sleep_for(chrono::milliseconds(1));
    }
    JsAppImpl::GetInstance().Stop();
}

static void InitSharedData()
{
    CommandParser& parser = CommandParser::GetInstance();
    if (parser.IsSet("l")) {
        SharedData<string>(SharedDataType::LANGUAGE, parser.Value("l"));
    } else {
        SharedData<string>(SharedDataType::LANGUAGE, "zh_CN");
    }
    string lanInfo = SharedData<string>::GetData(SharedDataType::LANGUAGE);
    SharedData<string>(SharedDataType::LAN, lanInfo.substr(0, lanInfo.find("_")));
    SharedData<string>(SharedDataType::REGION, lanInfo.substr(lanInfo.find("_") + 1, lanInfo.length() - 1));
    ILOG("Start language is : %s", SharedData<string>::GetData(SharedDataType::LANGUAGE).c_str());
}

int main(int argc, char* argv[])
{
    ILOG("RichPreviewer enter the main function.");
    auto richCrashHandler = std::make_unique<CrashHandler>();
    if (richCrashHandler == nullptr) {
        ELOG("RichPreviewer crashHandler new fail.");
        return 0;
    }
    // init exception handler
    richCrashHandler->InitExceptionHandler();
    // Parsing User Commands
    CommandParser& parser = CommandParser::GetInstance();
    vector<string> strs;
    for (int i = 1; i < argc; ++i) {
        if (parser.IsMainArgLengthInvalid(argv[i]))
            return START_PARAM_INVALID_CODE;
        strs.push_back(argv[i]);
    }

    if (!parser.ProcessCommand(strs)) {
        return 0;
    }
    // check params
    if (!parser.IsCommandValid()) {
        return START_PARAM_INVALID_CODE;
    }

    InitSharedData();
    if (parser.IsSet("s")) {
        CommandLineInterface::GetInstance().Init(parser.Value("s"));
    }

    TraceTool::GetInstance().HandleTrace("Enter the main function");

    std::thread commandThead(ProcessCommand);
    commandThead.detach();
    InitJsApp();
    this_thread::sleep_for(chrono::milliseconds(500));
    return 0;
}
