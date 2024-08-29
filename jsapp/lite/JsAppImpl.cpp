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

#include "JsAppImpl.h"

#include "engines/gfx/gfx_engine_manager.h"
#include "font/ui_font.h"
#include "font/ui_font_bitmap.h"
#include "font/ui_font_header.h"
#include "font/ui_font_vector.h"
#include "font/ui_line_break.h"
#include "font/ui_text_shaping.h"
#include "gfx_utils/color.h"
#include "gfx_utils/file.h"
#include "global.h"
#include "graphic_startup.h"
#include "input_device_manager.h"
#include "js_debugger_config.h"
#include "product_adapter.h"
#include "screen_device.h"

#include "AsyncWorkManager.h"
#include "CommandParser.h"
#include "CppTimerManager.h"
#include "FileSystem.h"
#include "LanguageManagerImpl.h"
#include "MouseInputImpl.h"
#include "MouseWheelImpl.h"
#include "PreviewerEngineLog.h"
#include "SharedData.h"
#include "TimerTaskHandler.h"
#include "TraceTool.h"
#include "VirtualScreenImpl.h"

using namespace OHOS;
using namespace ACELite;

static uint8_t g_fontPsramBaseAddr[MIN_FONT_PSRAM_LENGTH];

static void InitVectorFont(UIFont* font, const std::string fontPath)
{
    ProductAdapter::SetDefaultFontStyle("SourceHanSansSC-Regular.otf", JsAppImpl::FONT_SIZE_DEFAULT);
    BaseFont* currentFont = new UIFontVector();
    if (currentFont == nullptr) {
        FLOG("InitVectorFont currentFont memory allocation failed");
        return;
    }
    font->SetFont(currentFont);
    font->SetPsramMemory(reinterpret_cast<uintptr_t>(g_fontPsramBaseAddr), MIN_FONT_PSRAM_LENGTH);
    int8_t ret = font->SetFontPath(fontPath.data(), BaseFont::DYNAMIC_FONT);
    if (ret != 0) {
        ELOG("The vector font path does not exist ! fontPath : %s", fontPath.data());
        FLOG("InitFontEngine SetFontPath failed. vector fontPath: %s", fontPath.data());
    }
    font->RegisterFontInfo("SourceHanSansSC-Regular.otf");
    font->SetCurrentLangId(0);
}

static void InitFontEngine()
{
    UIFont* font = UIFont::GetInstance();
    if (font == nullptr) {
        ELOG("InitFontEngine:font is nullptr");
        return;
    }
    std::string deviceType = CommandParser::GetInstance().GetDeviceType();
    std::string separator = FileSystem::GetSeparator();
    std::string fontPath = FileSystem::GetApplicationPath() + separator + ".." + separator + "config" + separator;
    InitVectorFont(font, fontPath);

    int32_t fp = 0;
    std::string fileName = fontPath + "line_cj.brk";
#ifdef _WIN32
    fp = open(fileName.c_str(), O_RDONLY | O_BINARY);
#else
    fp = open(fileName.c_str(), O_RDONLY);
#endif
    if (fp < 0) {
        ELOG("Open font path failed.");
        return;
    }
    uint32_t lineBrkSize = lseek(fp, 0, SEEK_END);
    lseek(fp, 0, SEEK_SET);
    UILineBreakEngine& lbEngine = UILineBreakEngine::GetInstance();
    lbEngine.SetRuleBinInfo(fp, 0, lineBrkSize);
    lbEngine.Init();
}

static void InitHalScreen()
{
    InputDeviceManager::GetInstance()->Add(&MouseInputImpl::GetInstance());
    InputDeviceManager::GetInstance()->Add(&MouseWheelImpl::GetInstance());
}

JsAppImpl::JsAppImpl()
    : isInterrupt(false),
      taskHandleTimer(nullptr),
      deviceCheckTimer(nullptr),
      jsCheckTimer(nullptr),
      jsAbility(nullptr),
      jsThread(nullptr)
{
}

JsAppImpl::~JsAppImpl() {}

JsAppImpl& JsAppImpl::GetInstance()
{
    static JsAppImpl instance;
    return instance;
}

void JsAppImpl::Start()
{
    isFinished = false;
    isInterrupt = false;
    jsThread = std::make_unique<std::thread>([this]() {
        this->ThreadCallBack();
    });
    if (jsThread == nullptr) {
        ELOG("JsApp::Start jsThread memory allocation failed");
    }
    jsThread->detach();
}

void JsAppImpl::Restart()
{
    Stop();
    Start();
}

void JsAppImpl::Interrupt()
{
    AsyncWorkManager::GetInstance().ClearAllAsyncWork();
    jsAbility->Hide();
    jsAbility->TransferToDestroy();
    jsAbility.reset();
    isFinished = true;
    isInterrupt = true;
    ILOG("JsAppImpl::ThreadCallBack finished");
}

void JsAppImpl::ThreadCallBack()
{
    OHOS::GraphicStartUp::Init();
    GLOBAL_ConfigLanguage(SharedData<std::string>::GetData(SharedDataType::LANGUAGE).data());
    InitHalScreen();
    InitFontEngine();
    VirtualScreenImpl::GetInstance().InitAll(pipeName, pipePort);
    StartJsApp();
    InitTimer();

    std::thread::id curThreadId = std::this_thread::get_id();
#if defined(LITEWEARABLE_SUPPORTED) && LITEWEARABLE_SUPPORTED
    SharedData<uint8_t>::AppendNotify(SharedDataType::HEARTBEAT_VALUE, TimerTaskHandler::CheckHeartRateChanged,
        curThreadId, 50); // Duration:50 x 100 ms
    SharedData<uint32_t>::AppendNotify(SharedDataType::PRESSURE_VALUE, TimerTaskHandler::CheckBarometerChanged,
        curThreadId);
    SharedData<uint32_t>::AppendNotify(SharedDataType::SUMSTEP_VALUE, TimerTaskHandler::CheckStepCountChanged,
        curThreadId);
    SharedData<bool>::AppendNotify(SharedDataType::WEARING_STATE, TimerTaskHandler::CheckOnBodyStateChanged,
        curThreadId);
#endif
    SharedData<std::string>::AppendNotify(SharedDataType::LANGUAGE, TimerTaskHandler::CheckLanguageChanged,
        curThreadId);

    CppTimerManager& manager = CppTimerManager::GetTimerManager();
    while (!isInterrupt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        manager.RunTimerTick();
    }
}

void JsAppImpl::InitTimer()
{
    taskHandleTimer = std::make_unique<CppTimer>(TimerTaskHandler::TaskHandle);
    if (taskHandleTimer == nullptr) {
        ELOG("JsApp::InitTimer taskHandleTimer memory allocation failed.");
        return;
    }
    CppTimerManager::GetTimerManager().AddCppTimer(*taskHandleTimer);
    taskHandleTimer->Start(TASK_HANDLE_TIMER_INTERVAL);

    deviceCheckTimer = std::make_unique<CppTimer>(TimerTaskHandler::CheckDevice);
    if (deviceCheckTimer == nullptr) {
        ELOG("JsApp::InitTimer deviceCheckTimer memory allocation failed.");
        return;
    }
    CppTimerManager::GetTimerManager().AddCppTimer(*deviceCheckTimer);
    deviceCheckTimer->Start(DEVICE_CHECK_TIMER_INTERVAL);

    jsCheckTimer = std::make_unique<CppTimer>(TimerTaskHandler::CheckJsRunning);
    if (jsCheckTimer == nullptr) {
        ELOG("JsApp::InitTimer jsCheckTimer memory allocation failed.");
        return;
    }
    CppTimerManager::GetTimerManager().AddCppTimer(*jsCheckTimer);
    jsCheckTimer->Start(JS_CHECK_TIMER_INTERVAL);
}

void JsAppImpl::StartJsApp()
{
    if (jsAbility != nullptr) {
        FLOG("JsApp::StartJsApp jsAbility is not null.");
        return;
    }

    jsAbility = std::make_unique<OHOS::ACELite::JSAbility>();
    if (jsAbility == nullptr) {
        FLOG("JsApp::StartJsApp jsAbility memory allocation failed");
        return;
    }

    DebuggerConfig config;
    config.startDebuggerServer = isDebug;
    ILOG("Launch JS APP.");
    ILOG("Debug Server Enable: %d", config.startDebuggerServer);
    config.snapshotMode = false;
    config.heapSize = jsHeapSize;
    if (isDebug && debugServerPort) {
        config.port = debugServerPort;
        config.startDebuggerServer = isDebug;
        config.snapshotMode = false;
        ILOG("Debug Server Port: %d", debugServerPort);
    }
    Debugger::GetInstance().ConfigEngineDebugger(config);
    ILOG("Launch Js app");
    TraceTool::GetInstance().HandleTrace("Launch Js app");
    if (urlPath.empty()) {
        jsAbility->Launch(jsAppPath.c_str(), bundleName.c_str(), 0);
        jsAbility->Show();
        ILOG("JsApp::StartJsApp launch finished.");
        return;
    }
    Json2::Value val = JsonReader::CreateObject();
    val.Add("uri", urlPath.c_str());
    std::string routerInfo = val.ToStyledString();
    jsAbility->Launch(jsAppPath.c_str(), bundleName.c_str(), 0, routerInfo.data());
    jsAbility->Show();
    ILOG("JsApp::StartJsApp launch with single page mode finished.");
    isFinished = false;
}

void JsAppImpl::InitJsApp()
{
    CommandParser& parser = CommandParser::GetInstance();
    // Initialize Image Pipeline Name
    if (parser.IsSet("s")) {
        SetPipeName(parser.Value("s"));
    }
    if (parser.IsSet("lws")) {
        SetPipePort(parser.Value("lws"));
    }
    // Set the application name.
    SetBundleName(parser.GetAppName());
    // Processing JSheap
    SetJSHeapSize(parser.GetJsHeapSize());
    // Start JSApp
    if (!parser.IsSet("t")) {
        if (parser.IsSet("d")) {
            SetIsDebug(true);
            if (parser.IsSet("p")) {
                SetDebugServerPort(static_cast<uint16_t>(atoi(parser.Value("p").c_str())));
            }
        }
        SetJsAppPath(parser.Value("j"));
        if (parser.IsSet("url")) {
            SetUrlPath(parser.Value("url"));
        }
        Start();
    }
}
