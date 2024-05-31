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

using namespace std;
static const int NOTIFY_INTERVAL_TIME = 1000; // Unit millisecond

static void ApplyConfig()
{
    string richConfigArgs = CommandParser::GetInstance().GetConfigPath();
    if (richConfigArgs.empty()) {
        ELOG("No persistent properties path found.");
    }
    CommandLineInterface::GetInstance().ReadAndApplyConfig(richConfigArgs);
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
    Json2::Value commandResult = JsonReader::CreateObject();
    commandResult.Add("version", CommandLineInterface::COMMAND_VERSION.c_str());
    commandResult.Add("command", "inspector");
    commandResult.Add("result", jsonTree.c_str());
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
    int ret = parser.ParseArgs(argc, argv);
    if (ret >= 0) {
        return ret;
    }
    InitSharedData();
    if (parser.IsSet("s")) {
        CommandLineInterface::GetInstance().Init(parser.Value("s"));
    }

    TraceTool::GetInstance().HandleTrace("Enter the main function");

    std::thread commandThead(ProcessCommand);
    commandThead.detach();
    VirtualScreenImpl::GetInstance().InitResolution();
    ApplyConfig();
    JsAppImpl::GetInstance().InitJsApp();
    this_thread::sleep_for(chrono::milliseconds(500)); // sleep 500 ms
    return 0;
}
