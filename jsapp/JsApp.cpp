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

#include "JsApp.h"
#include <algorithm>
#include <sstream>
#include <cstdio>
#include "FileSystem.h"
#include "PreviewerEngineLog.h"

using namespace std;

static std::vector<std::string> liteDevice = {"liteWearable", "smartVision"};

JsApp::JsApp()
    : pipeName(""),
      pipePort(""),
      jsAppPath(""),
      bundleName("undefined"),
      urlPath(""),
      isFinished(true),
      isRunning(true),
      isDebug(false),
      debugServerPort(0),
      jsHeapSize(0),
      colorMode(""),
      orientation(""),
      aceVersion(""),
      screenDensity(""),
      configChanges("")
{
}

void JsApp::Stop()
{
    ILOG("JsApp::Stop start stop js app.");
    auto start = std::chrono::system_clock::now();
    while (!isFinished) {
        Interrupt();
        this_thread::sleep_for(chrono::milliseconds(1));
        auto end = std::chrono::system_clock::now();
        auto passedSecond = chrono::duration_cast<chrono::seconds>(end - start).count();
        if (passedSecond > 10) { // The restart timeout interval is 10 seconds.
            ILOG("Restart js app time out!");
            return;
        }
    }
    ILOG("JsApp::Stop js app stop finished.");
}

void JsApp::Interrupt() {}

bool JsApp::IsLiteDevice(std::string deviceType)
{
    auto iter = find(liteDevice.begin(), liteDevice.end(), deviceType);
    if (iter == liteDevice.end()) {
        return false;
    }
    return true;
}

void JsApp::SetPipeName(const std::string& name)
{
    pipeName = name;
}

void JsApp::SetPipePort(const std::string& port)
{
    pipePort = port;
}

void JsApp::SetJsAppPath(const string& value)
{
    jsAppPath = value;
}

void JsApp::SetScreenDensity(const std::string value)
{
    screenDensity = value;
}

void JsApp::SetConfigChanges(const std::string value)
{
    configChanges = value;
}

void JsApp::SetUrlPath(const string& value)
{
    urlPath = value;
}

void JsApp::SetBundleName(const string& name)
{
    bundleName = name;
    FileSystem::SetBundleName(name);
    FileSystem::MakeVirtualFileSystemPath();
}

void JsApp::SetRunning(bool flag)
{
    isRunning = flag;
}

bool JsApp::GetRunning()
{
    return isRunning;
}

void JsApp::SetIsDebug(bool value)
{
    isDebug = value;
}

void JsApp::SetDebugServerPort(uint16_t value)
{
    debugServerPort = value;
}

void JsApp::SetJSHeapSize(uint32_t size)
{
    jsHeapSize = size;
}

string JsApp::GetJSONTree()
{
    return "";
}

string JsApp::GetDefaultJSONTree()
{
    return "";
}

void JsApp::SetArgsColorMode(const string& value)
{
    colorMode = value;
}

void JsApp::SetArgsAceVersion(const string& value)
{
    aceVersion = value;
}

void JsApp::ColorModeChanged(const std::string commandColorMode)
{
    colorMode = commandColorMode;
}

std::string JsApp::GetColorMode() const
{
    return colorMode;
}

std::string JsApp::GetOrientation() const
{
    return orientation;
}

void JsApp::OrientationChanged(std::string commandOrientation)
{
    orientation = commandOrientation;
}

void JsApp::ResolutionChanged(ResolutionParam&, int32_t, std::string) {}

void JsApp::ReloadRuntimePage(const std::string) {}

bool JsApp::MemoryRefresh(const std::string) const
{
    return false;
}

void JsApp::LoadDocument(const std::string, const std::string, const Json2::Value&) {}

void JsApp::FoldStatusChanged(const std::string commandFoldStatus, int32_t width, int32_t height) {}

void JsApp::SetAvoidArea(const AvoidAreas& areas) {}

const AvoidAreas JsApp::GetCurrentAvoidArea() const
{
    AvoidAreas areas;
    return areas;
}

void JsApp::InitJsApp() {}