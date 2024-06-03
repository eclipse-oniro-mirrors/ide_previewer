/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <string>
#include <sys/stat.h>
#include "gtest/gtest.h"
#include "window.h"
#define private public
#define protected public
#include "MockGlobalResult.h"
#include "JsAppImpl.h"
#include "CommandLineInterface.h"
#include "CommandParser.h"
#include "VirtualScreenImpl.h"
#include "window_model.h"
#include "window_display.h"
#include "FileSystem.h"
using namespace std;

namespace {
    class JsAppImplTest : public ::testing::Test {
    public:
        JsAppImplTest() {}
        ~JsAppImplTest() {}
    protected:
        static void WriteFile(std::string testFile, std::string fileContent)
        {
            std::ofstream file(testFile, std::ios::out | std::ios::in | std::ios_base::trunc);
            if (file.is_open()) {
                file << fileContent;
                file.close();
            } else {
                printf("Error open file!\n");
            }
        }

        static void SetUpTestCase()
        {
            CommandLineInterface::GetInstance().Init("pipeName");
            char buffer[FILENAME_MAX];
            if (getcwd(buffer, FILENAME_MAX) != nullptr) {
                testDir = std::string(buffer);
            } else {
                printf("error: getcwd failed\n");
            }
        }

        static std::string testDir;
    };
    std::string JsAppImplTest::testDir = "";

    // 测试拷贝构造函数是否被删除
    TEST_F(JsAppImplTest, CopyConstructorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_constructible<JsApp>::value == false);
    }

    // 测试赋值运算符是否被删除
    TEST_F(JsAppImplTest, AssignmentOperatorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_assignable<JsApp>::value == false);
    }

    TEST_F(JsAppImplTest, StartTest)
    {
        JsAppImpl::GetInstance().isStop = false;
        std::thread thread1([]() {
            JsAppImpl::GetInstance().Start();
        });
        thread1.detach();
        this_thread::sleep_for(chrono::milliseconds(10));
        JsAppImpl::GetInstance().isStop = true;
        this_thread::sleep_for(chrono::milliseconds(10));
        EXPECT_TRUE(JsAppImpl::GetInstance().isFinished);
    }

    TEST_F(JsAppImplTest, InitJsAppTest)
    {
        JsAppImpl::GetInstance().isStop = false;
        std::thread thread1([]() {
            JsAppImpl::GetInstance().InitJsApp();
        });
        thread1.detach();
        this_thread::sleep_for(chrono::milliseconds(10));
        JsAppImpl::GetInstance().isStop = true;
        this_thread::sleep_for(chrono::milliseconds(10));
        EXPECT_TRUE(JsAppImpl::GetInstance().isFinished);
    }

    TEST_F(JsAppImplTest, RestartTest)
    {
        JsAppImpl::GetInstance().ability =
            OHOS::Ace::Platform::AceAbility::CreateInstance(JsAppImpl::GetInstance().aceRunArgs);
        JsAppImpl::GetInstance().Restart();
        bool eq = JsAppImpl::GetInstance().ability == nullptr;
        EXPECT_TRUE(eq);
    }

    TEST_F(JsAppImplTest, InterruptTest)
    {
        JsAppImpl::GetInstance().isStop = false;
        JsAppImpl::GetInstance().Interrupt();
        EXPECT_TRUE(JsAppImpl::GetInstance().isStop);
    }

    TEST_F(JsAppImplTest, GetJSONTreeTest)
    {
        EXPECT_EQ(JsAppImpl::GetInstance().GetJSONTree(), "jsontree");
    }

    TEST_F(JsAppImplTest, GetDefaultJSONTreeTest)
    {
        EXPECT_EQ(JsAppImpl::GetInstance().GetDefaultJSONTree(), "defaultjsontree");
    }

    TEST_F(JsAppImplTest, OrientationChangedTest)
    {
        JsAppImpl::GetInstance().ability =
            OHOS::Ace::Platform::AceAbility::CreateInstance(JsAppImpl::GetInstance().aceRunArgs);
        g_surfaceChanged = false;
        JsAppImpl::GetInstance().OrientationChanged("portrait");
        EXPECT_TRUE(g_surfaceChanged);
        EXPECT_EQ(JsAppImpl::GetInstance().orientation, "portrait");
        g_surfaceChanged = false;
        JsAppImpl::GetInstance().OrientationChanged("landscape");
        EXPECT_TRUE(g_surfaceChanged);
        EXPECT_EQ(JsAppImpl::GetInstance().orientation, "landscape");
    }

    TEST_F(JsAppImplTest, ResolutionChangedTest)
    {
        JsAppImpl::GetInstance().isStop = false;
        std::thread thread1([]() {
            JsAppImpl::GetInstance().Start();
        });
        thread1.detach();
        this_thread::sleep_for(chrono::milliseconds(10));
        int32_t originWidth = 222;
        int32_t originHeight = 333;
        int32_t width = 222;
        int32_t height = 333;
        int32_t screenDensity = 360;
        string reason = "resize";
        ResolutionParam param(originWidth, originHeight, width, height);
        g_surfaceChanged = false;
        JsAppImpl::GetInstance().ResolutionChanged(param, screenDensity, reason);
        EXPECT_TRUE(g_surfaceChanged);
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceWidth, 222);
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceHeight, 333);
    }

    TEST_F(JsAppImplTest, ConvertResizeReasonTest)
    {
        EXPECT_EQ(JsAppImpl::GetInstance().ConvertResizeReason("undefined"),
            OHOS::Ace::WindowSizeChangeReason::UNDEFINED);
        EXPECT_EQ(JsAppImpl::GetInstance().ConvertResizeReason("rotation"),
            OHOS::Ace::WindowSizeChangeReason::ROTATION);
        EXPECT_EQ(JsAppImpl::GetInstance().ConvertResizeReason("resize"),
            OHOS::Ace::WindowSizeChangeReason::RESIZE);
    }

    TEST_F(JsAppImplTest, SetResolutionParamsTest)
    {
        CommandParser::GetInstance().deviceType = "phone";
        JsAppImpl::GetInstance().InitCommandInfo();
        int32_t originWidth = 111;
        int32_t originHeight = 222;
        int32_t width = 111;
        int32_t height = 222;
        int32_t screenDensity = 480;
        JsAppImpl::GetInstance().SetResolutionParams(originWidth, originHeight, width, height, screenDensity);
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceWidth, 111);
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceHeight, 222);
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceConfig.density, 3);
        EXPECT_EQ(JsAppImpl::GetInstance().orientation, "portrait");
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceConfig.orientation,
            OHOS::Ace::DeviceOrientation::PORTRAIT);

        originWidth = 222;
        originHeight = 111;
        width = 222;
        height = 111;
        screenDensity = 320;
        JsAppImpl::GetInstance().SetResolutionParams(originWidth, originHeight, width, height, screenDensity);
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceWidth, 222);
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceHeight, 111);
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceConfig.density, 2);
        EXPECT_EQ(JsAppImpl::GetInstance().orientation, "landscape");
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceConfig.orientation,
            OHOS::Ace::DeviceOrientation::LANDSCAPE);
    }

    TEST_F(JsAppImplTest, SetArgsColorModeTest)
    {
        JsAppImpl::GetInstance().SetArgsColorMode("light");
        EXPECT_EQ("light", JsAppImpl::GetInstance().colorMode);
    }

    TEST_F(JsAppImplTest, SetArgsAceVersionTest)
    {
        JsAppImpl::GetInstance().SetArgsAceVersion("ACE_2_0");
        EXPECT_EQ("ACE_2_0", JsAppImpl::GetInstance().aceVersion);
    }

    TEST_F(JsAppImplTest, SetDeviceOrentationTest)
    {
        JsAppImpl::GetInstance().SetDeviceOrentation("landscape");
        EXPECT_EQ("landscape", JsAppImpl::GetInstance().orientation);
    }

    TEST_F(JsAppImplTest, GetOrientationTest)
    {
        JsAppImpl::GetInstance().orientation = "portrait";
        EXPECT_EQ(JsAppImpl::GetInstance().GetOrientation(), "portrait");
    }

    TEST_F(JsAppImplTest, GetColorModeTest)
    {
        JsAppImpl::GetInstance().colorMode = "dark";
        EXPECT_EQ(JsAppImpl::GetInstance().GetColorMode(), "dark");
    }

    TEST_F(JsAppImplTest, ColorModeChangedTest)
    {
        JsAppImpl::GetInstance().ability =
            OHOS::Ace::Platform::AceAbility::CreateInstance(JsAppImpl::GetInstance().aceRunArgs);
        g_onConfigurationChanged = false;
        JsAppImpl::GetInstance().ColorModeChanged("light");
        EXPECT_TRUE(g_onConfigurationChanged);
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceConfig.colorMode,
            OHOS::Ace::ColorMode::LIGHT);
    }

    TEST_F(JsAppImplTest, ReloadRuntimePageTest)
    {
        JsAppImpl::GetInstance().ability =
            OHOS::Ace::Platform::AceAbility::CreateInstance(JsAppImpl::GetInstance().aceRunArgs);
        g_replacePage = false;
        JsAppImpl::GetInstance().ReloadRuntimePage("pages/Index");
        EXPECT_TRUE(g_replacePage);
    }

    TEST_F(JsAppImplTest, SetScreenDensityTest)
    {
        JsAppImpl::GetInstance().SetScreenDensity("360");
        EXPECT_EQ(JsAppImpl::GetInstance().screenDensity, "360");
    }

    TEST_F(JsAppImplTest, SetConfigChangesTest)
    {
        JsAppImpl::GetInstance().SetConfigChanges("aaa");
        EXPECT_EQ(JsAppImpl::GetInstance().configChanges, "aaa");
    }

    TEST_F(JsAppImplTest, MemoryRefreshTest)
    {
        JsAppImpl::GetInstance().ability =
            OHOS::Ace::Platform::AceAbility::CreateInstance(JsAppImpl::GetInstance().aceRunArgs);
        g_operateComponent = false;
        JsAppImpl::GetInstance().MemoryRefresh("aaa");
        EXPECT_TRUE(g_operateComponent);
    }

    TEST_F(JsAppImplTest, LoadDocumentTest)
    {
        JsAppImpl::GetInstance().ability =
            OHOS::Ace::Platform::AceAbility::CreateInstance(JsAppImpl::GetInstance().aceRunArgs);
        Json2::Value val = JsonReader::CreateNull();
        g_loadAceDocument = false;
        JsAppImpl::GetInstance().LoadDocument("aaa", "bbb", val);
        EXPECT_TRUE(g_loadAceDocument);
    }

    TEST_F(JsAppImplTest, FoldStatusChangedTest)
    {
        JsAppImpl::GetInstance().isStop = false;
        std::thread thread1([]() {
            JsAppImpl::GetInstance().Start();
        });
        thread1.detach();
        this_thread::sleep_for(chrono::milliseconds(10));
        int width = 200;
        int height = 300;
        g_execStatusChangedCallback = false;
        JsAppImpl::GetInstance().FoldStatusChanged("fold", width, height);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().foldStatus, "fold");
        EXPECT_EQ(OHOS::Previewer::PreviewerDisplay::GetInstance().foldStatus_, OHOS::Rosen::FoldStatus::FOLDED);
        EXPECT_TRUE(g_execStatusChangedCallback);
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceWidth, width);
        EXPECT_EQ(JsAppImpl::GetInstance().aceRunArgs.deviceHeight, height);

        JsAppImpl::GetInstance().FoldStatusChanged("unfold", width, height);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().foldStatus, "unfold");
        EXPECT_EQ(OHOS::Previewer::PreviewerDisplay::GetInstance().foldStatus_, OHOS::Rosen::FoldStatus::EXPAND);

        JsAppImpl::GetInstance().FoldStatusChanged("half_fold", width, height);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().foldStatus, "half_fold");
        EXPECT_EQ(OHOS::Previewer::PreviewerDisplay::GetInstance().foldStatus_, OHOS::Rosen::FoldStatus::HALF_FOLD);

        width = 210;
        height = 310;
        g_execStatusChangedCallback = false;
        JsAppImpl::GetInstance().FoldStatusChanged("unknown", width, height);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().foldStatus, "unknown");
        EXPECT_EQ(OHOS::Previewer::PreviewerDisplay::GetInstance().foldStatus_, OHOS::Rosen::FoldStatus::UNKNOWN);
        EXPECT_TRUE(g_execStatusChangedCallback);
        EXPECT_NE(JsAppImpl::GetInstance().aceRunArgs.deviceWidth, width);
        EXPECT_NE(JsAppImpl::GetInstance().aceRunArgs.deviceHeight, height);
    }

    TEST_F(JsAppImplTest, DispatchBackPressedEventTest)
    {
        g_onBackPressed = false;
        JsAppImpl::GetInstance().DispatchBackPressedEvent();
        EXPECT_TRUE(g_onBackPressed);
    }

    TEST_F(JsAppImplTest, DispatchKeyEventTest)
    {
        g_onInputEvent = false;
        JsAppImpl::GetInstance().DispatchKeyEvent(nullptr);
        EXPECT_TRUE(g_onInputEvent);
    }

    TEST_F(JsAppImplTest, DispatchPointerEventTest)
    {
        g_onInputEvent = false;
        JsAppImpl::GetInstance().DispatchPointerEvent(nullptr);
        EXPECT_TRUE(g_onInputEvent);
    }

    TEST_F(JsAppImplTest, DispatchAxisEventTest)
    {
        g_onInputEvent = false;
        JsAppImpl::GetInstance().DispatchAxisEvent(nullptr);
        EXPECT_TRUE(g_onInputEvent);
    }

    TEST_F(JsAppImplTest, DispatchInputMethodEventTest)
    {
        g_onInputMethodEvent = false;
        int code = 12;
        JsAppImpl::GetInstance().DispatchInputMethodEvent(code);
        EXPECT_TRUE(g_onInputMethodEvent);
    }

    TEST_F(JsAppImplTest, InitGlfwEnvTest)
    {
        g_glfwInit = false;
        g_createGlfwWindow = false;
        JsAppImpl::GetInstance().InitGlfwEnv();
        EXPECT_TRUE(g_glfwInit);
        EXPECT_TRUE(g_createGlfwWindow);
    }

    TEST_F(JsAppImplTest, SetJsAppArgsTest)
    {
        CommandParser::GetInstance().isComponentMode = true;
        JsAppImpl::GetInstance().InitCommandInfo();
        OHOS::Ace::Platform::AceRunArgs args;
        JsAppImpl::GetInstance().SetJsAppArgs(args);
        EXPECT_EQ(args.windowTitle, "Ace");
        EXPECT_TRUE(args.isComponentMode);
    }

    TEST_F(JsAppImplTest, RunJsAppImplTest)
    {
        JsAppImpl::GetInstance().isDebug = false;
        JsAppImpl::GetInstance().debugServerPort = 0;
        g_setContentInfoCallback = false;
        g_createSurfaceNode = false;
        g_setWindow = false;
        g_initEnv = false;
        JsAppImpl::GetInstance().RunJsApp();
        EXPECT_TRUE(g_setContentInfoCallback);
        EXPECT_TRUE(g_createSurfaceNode);
        EXPECT_TRUE(g_setWindow);
        EXPECT_TRUE(g_initEnv);

        JsAppImpl::GetInstance().isDebug = true;
        JsAppImpl::GetInstance().debugServerPort = 8888;
        g_setHostResolveBufferTracker = false;
        g_setContentInfoCallback = false;
        g_createSurfaceNode = false;
        JsAppImpl::GetInstance().RunJsApp();
        EXPECT_FALSE(g_setHostResolveBufferTracker);
        EXPECT_FALSE(g_setContentInfoCallback);
        EXPECT_FALSE(g_createSurfaceNode);
    }

    // JsApp start
    TEST_F(JsAppImplTest, ResolutionParamTest)
    {
        int width = 100;
        ResolutionParam param(width, width, width, width);
        EXPECT_EQ(param.orignalWidth, width);
        EXPECT_EQ(param.orignalHeight, width);
        EXPECT_EQ(param.compressionWidth, width);
        EXPECT_EQ(param.compressionHeight, width);
    }

    TEST_F(JsAppImplTest, StopTest)
    {
        JsAppImpl::GetInstance().isStop = false;
        JsAppImpl::GetInstance().isFinished = false;
        std::thread thread1([]() {
            JsAppImpl::GetInstance().Stop();
        });
        thread1.detach();
        this_thread::sleep_for(chrono::milliseconds(10));
        JsAppImpl::GetInstance().isFinished = true;
        this_thread::sleep_for(chrono::milliseconds(20));
        EXPECT_TRUE(JsAppImpl::GetInstance().isStop);
    }

    TEST_F(JsAppImplTest, SetJsAppPathTest)
    {
        JsAppImpl::GetInstance().SetJsAppPath("pages/Index");
        EXPECT_EQ(JsAppImpl::GetInstance().jsAppPath, "pages/Index");
    }

    TEST_F(JsAppImplTest, SetUrlPathTest)
    {
        JsAppImpl::GetInstance().SetUrlPath("pages/Index");
        EXPECT_EQ(JsAppImpl::GetInstance().urlPath, "pages/Index");
    }

    TEST_F(JsAppImplTest, SetPipeNameTest)
    {
        JsAppImpl::GetInstance().SetPipeName("phone_1");
        EXPECT_EQ(JsAppImpl::GetInstance().pipeName, "phone_1");
    }

    TEST_F(JsAppImplTest, SetPipePortTest)
    {
        JsAppImpl::GetInstance().SetPipePort("5000");
        EXPECT_EQ(JsAppImpl::GetInstance().pipePort, "5000");
    }

    TEST_F(JsAppImplTest, SetBundleNameTest)
    {
        JsAppImpl::GetInstance().SetBundleName("aaa");
        EXPECT_EQ(JsAppImpl::GetInstance().bundleName, "aaa");
        EXPECT_EQ(FileSystem::bundleName, "aaa");
        size_t pos = FileSystem::fileSystemPath.find("aaa");
        EXPECT_TRUE(pos != std::string::npos);
    }

    TEST_F(JsAppImplTest, SetRunningTest)
    {
        JsAppImpl::GetInstance().SetRunning(true);
        EXPECT_TRUE(JsAppImpl::GetInstance().isRunning);
        JsAppImpl::GetInstance().SetRunning(false);
        EXPECT_FALSE(JsAppImpl::GetInstance().isRunning);
    }

    TEST_F(JsAppImplTest, GetRunningTest)
    {
        JsAppImpl::GetInstance().isRunning = true;
        EXPECT_TRUE(JsAppImpl::GetInstance().GetRunning());
        JsAppImpl::GetInstance().isRunning = false;
        EXPECT_FALSE(JsAppImpl::GetInstance().GetRunning());
    }

    TEST_F(JsAppImplTest, SetIsDebugTest)
    {
        JsAppImpl::GetInstance().SetIsDebug(true);
        EXPECT_TRUE(JsAppImpl::GetInstance().isDebug);
        JsAppImpl::GetInstance().SetIsDebug(false);
        EXPECT_FALSE(JsAppImpl::GetInstance().isDebug);
    }

    TEST_F(JsAppImplTest, SetDebugServerPortTest)
    {
        int port = 5000;
        JsAppImpl::GetInstance().SetDebugServerPort(port);
        EXPECT_EQ(JsAppImpl::GetInstance().debugServerPort, port);
    }

    TEST_F(JsAppImplTest, SetJSHeapSizeTest)
    {
        int size = 5000;
        JsAppImpl::GetInstance().SetJSHeapSize(size);
        EXPECT_EQ(JsAppImpl::GetInstance().jsHeapSize, size);
    }

    TEST_F(JsAppImplTest, IsLiteDeviceTest)
    {
        EXPECT_TRUE(JsApp::IsLiteDevice("liteWearable"));
        EXPECT_FALSE(JsApp::IsLiteDevice("phone"));
    }
    // JsApp end

    TEST_F(JsAppImplTest, SetPkgContextInfoTest)
    {
        CommandParser::GetInstance().appResourcePath = testDir;
        CommandParser::GetInstance().loaderJsonPath = testDir + "/loader.json";
        JsAppImpl::GetInstance().InitCommandInfo();
        const string moduleJsonPath = testDir + FileSystem::GetSeparator() + "module.json";
        const string pkgContextInfoJsonPath = testDir + FileSystem::GetSeparator() + "pkgContextInfo.json";
        // 直接调用
        JsAppImpl::GetInstance().SetPkgContextInfo();
        EXPECT_TRUE(JsAppImpl::GetInstance().aceRunArgs.packageNameList.empty());
        // 创建module.json,json不写全1
        std::string moduleJsonContent = R"({"aaa":"bbb"})";
        WriteFile(moduleJsonPath, moduleJsonContent);
        JsAppImpl::GetInstance().SetPkgContextInfo();
        EXPECT_TRUE(JsAppImpl::GetInstance().aceRunArgs.packageNameList.empty());
        // 创建module.json,json不写全2
        moduleJsonContent = R"({"module":{"name":333}})";
        WriteFile(moduleJsonPath, moduleJsonContent);
        JsAppImpl::GetInstance().SetPkgContextInfo();
        EXPECT_TRUE(JsAppImpl::GetInstance().aceRunArgs.packageNameList.empty());
        // 创建module.json,json写全
        moduleJsonContent = R"({"module":{"name":"entry","packageName":"entry"}})";
        WriteFile(moduleJsonPath, moduleJsonContent);
        JsAppImpl::GetInstance().SetPkgContextInfo();
        EXPECT_FALSE(JsAppImpl::GetInstance().aceRunArgs.packageNameList.empty());
        EXPECT_TRUE(JsAppImpl::GetInstance().aceRunArgs.pkgContextInfoJsonStringMap.empty());
        // 创建pkgContextInfo.json
        std::string pkgContextInfoJsonContent = R"({"entry":{"packageName":"entry"}})";
        WriteFile(pkgContextInfoJsonPath, pkgContextInfoJsonContent);
        JsAppImpl::GetInstance().SetPkgContextInfo();
        EXPECT_FALSE(JsAppImpl::GetInstance().aceRunArgs.packageNameList.empty());
        EXPECT_FALSE(JsAppImpl::GetInstance().aceRunArgs.pkgContextInfoJsonStringMap.empty());
        if (std::remove(moduleJsonPath.c_str()) != 0) {
            printf("Error deleting module.json file!\n");
        }
        if (std::remove(pkgContextInfoJsonPath.c_str()) != 0) {
            printf("Error deleting pkgContextInfo.json file!\n");
        }
    }

    TEST_F(JsAppImplTest, SetAvoidAreaTest)
    {
        AvoidAreas areas;
        JsAppImpl::GetInstance().SetAvoidArea(areas);
        bool ret = JsAppImpl::GetInstance().avoidInitialAreas == areas;
        EXPECT_TRUE(ret);
    }

    TEST_F(JsAppImplTest, UpdateAvoidArea2IdeTest)
    {
        const OHOS::Rosen::Rect value = {0, 0, 0, 0};
        g_output = false;
        JsAppImpl::GetInstance().UpdateAvoidArea2Ide("topRect", value);
        EXPECT_TRUE(g_output);
    }

    TEST_F(JsAppImplTest, GetWindowTest)
    {
        JsAppImpl::GetInstance().isDebug = true;
        OHOS::Rosen::Window* win1 = JsAppImpl::GetInstance().GetWindow();
        EXPECT_TRUE(win1 == nullptr);

        JsAppImpl::GetInstance().isDebug = false;
        JsAppImpl::GetInstance().ability =
            OHOS::Ace::Platform::AceAbility::CreateInstance(JsAppImpl::GetInstance().aceRunArgs);
        OHOS::Rosen::WMError errCode;
        OHOS::sptr<OHOS::Rosen::WindowOption> sp = nullptr;
        auto window = OHOS::Rosen::Window::Create("previewer", sp, nullptr, errCode);
        JsAppImpl::GetInstance().ability->SetWindow(window);
        OHOS::Rosen::Window* win2 = JsAppImpl::GetInstance().GetWindow();
        EXPECT_TRUE(window == win2);
    }

    TEST_F(JsAppImplTest, InitAvoidAreasTest)
    {
        OHOS::Rosen::WMError errCode;
        OHOS::sptr<OHOS::Rosen::WindowOption> sp = nullptr;
        auto window = OHOS::Rosen::Window::Create("previewer", sp, nullptr, errCode);
        g_getSystemBarPropertyByType = false;
        JsAppImpl::GetInstance().InitAvoidAreas(window);
        EXPECT_TRUE(g_getSystemBarPropertyByType);
    }
}