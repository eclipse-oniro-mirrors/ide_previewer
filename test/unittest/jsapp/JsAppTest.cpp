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
#include <cstdint>
#include <string>
#include "gtest/gtest.h"
#define private public
#define protected public
#include "JsApp.h"
#include "FileSystem.h"

namespace {
    // JsApp 为抽象基类，通过最小化子类只重写纯虚接口，测试 JsApp.cpp 的基类实现
    class TestJsApp : public JsApp {
    public:
        TestJsApp() : JsApp(), interruptCount(0), finishAfterInterrupt(1) {}
        ~TestJsApp() override = default;

        void Start() override {}
        void Restart() override {}
        void Interrupt() override
        {
            interruptCount++;
            if (static_cast<int32_t>(interruptCount) >= finishAfterInterrupt) {
                isFinished = true;
            }
        }

        uint32_t interruptCount;
        int32_t finishAfterInterrupt;
    };

    class JsAppTest : public ::testing::Test {
    public:
        JsAppTest() {}
        ~JsAppTest() {}
    };

    TEST_F(JsAppTest, ConstructorDefaultsTest)
    {
        TestJsApp app;
        EXPECT_EQ(app.pipeName, "");
        EXPECT_EQ(app.pipePort, "");
        EXPECT_EQ(app.jsAppPath, "");
        EXPECT_EQ(app.bundleName, "undefined");
        EXPECT_EQ(app.urlPath, "");
        EXPECT_TRUE(app.isFinished.load());
        EXPECT_TRUE(app.isRunning.load());
        EXPECT_FALSE(app.isDebug);
        EXPECT_EQ(app.debugServerPort, 0);
        EXPECT_EQ(app.jsHeapSize, 0);
        EXPECT_EQ(app.colorMode, "");
        EXPECT_EQ(app.orientation, "");
        EXPECT_EQ(app.aceVersion, "");
        EXPECT_EQ(app.screenDensity, "");
        EXPECT_EQ(app.configChanges, "");
        EXPECT_TRUE(app.GetRunning());
    }

    TEST_F(JsAppTest, IsLiteDeviceTest)
    {
        EXPECT_TRUE(JsApp::IsLiteDevice("liteWearable"));
        EXPECT_TRUE(JsApp::IsLiteDevice("smartVision"));
        EXPECT_FALSE(JsApp::IsLiteDevice("phone"));
        EXPECT_FALSE(JsApp::IsLiteDevice("tablet"));
        EXPECT_FALSE(JsApp::IsLiteDevice(""));
        EXPECT_FALSE(JsApp::IsLiteDevice("LiteWearable"));
    }

    TEST_F(JsAppTest, StopFinishedImmediatelyTest)
    {
        TestJsApp app;
        // isFinished 默认为 true，Stop 应直接返回且不触发 Interrupt
        app.Stop();
        EXPECT_EQ(app.interruptCount, 0);
        EXPECT_TRUE(app.isFinished.load());
    }

    TEST_F(JsAppTest, StopWithInterruptLoopTest)
    {
        TestJsApp app;
        app.isFinished = false;
        app.finishAfterInterrupt = 3;
        app.Stop();
        // Interrupt 被调用 3 次后将 isFinished 置 true，Stop 退出循环
        EXPECT_EQ(app.interruptCount, 3);
        EXPECT_TRUE(app.isFinished.load());
    }

    TEST_F(JsAppTest, InterruptBaseImplTest)
    {
        TestJsApp app;
        // 基类 Interrupt 为空实现，直接调用不应有副作用
        app.JsApp::Interrupt();
        EXPECT_EQ(app.interruptCount, 0);
        app.Interrupt();
        EXPECT_EQ(app.interruptCount, 1);
    }

    TEST_F(JsAppTest, SetPipeNameTest)
    {
        TestJsApp app;
        app.SetPipeName("pipe_ut");
        EXPECT_EQ(app.pipeName, "pipe_ut");
    }

    TEST_F(JsAppTest, SetPipePortTest)
    {
        TestJsApp app;
        app.SetPipePort("5000");
        EXPECT_EQ(app.pipePort, "5000");
    }

    TEST_F(JsAppTest, SetJsAppPathTest)
    {
        TestJsApp app;
        app.SetJsAppPath("pages/Index");
        EXPECT_EQ(app.jsAppPath, "pages/Index");
    }

    TEST_F(JsAppTest, SetUrlPathTest)
    {
        TestJsApp app;
        app.SetUrlPath("pages/Index");
        EXPECT_EQ(app.urlPath, "pages/Index");
    }

    TEST_F(JsAppTest, SetScreenDensityTest)
    {
        TestJsApp app;
        app.SetScreenDensity("480");
        EXPECT_EQ(app.screenDensity, "480");
    }

    TEST_F(JsAppTest, SetConfigChangesTest)
    {
        TestJsApp app;
        app.SetConfigChanges("colormode_dark");
        EXPECT_EQ(app.configChanges, "colormode_dark");
    }

    TEST_F(JsAppTest, SetBundleNameTest)
    {
        TestJsApp app;
        app.SetBundleName("com.example.ut");
        EXPECT_EQ(app.bundleName, "com.example.ut");
        EXPECT_EQ(FileSystem::bundleName, "com.example.ut");
    }

    TEST_F(JsAppTest, SetRunningAndGetRunningTest)
    {
        TestJsApp app;
        app.SetRunning(false);
        EXPECT_FALSE(app.GetRunning());
        app.SetRunning(true);
        EXPECT_TRUE(app.GetRunning());
    }

    TEST_F(JsAppTest, SetIsDebugTest)
    {
        TestJsApp app;
        app.SetIsDebug(true);
        EXPECT_TRUE(app.isDebug);
        app.SetIsDebug(false);
        EXPECT_FALSE(app.isDebug);
    }

    TEST_F(JsAppTest, SetDebugServerPortTest)
    {
        TestJsApp app;
        app.SetDebugServerPort(8888);
        EXPECT_EQ(app.debugServerPort, 8888);
    }

    TEST_F(JsAppTest, SetJSHeapSizeTest)
    {
        TestJsApp app;
        app.SetJSHeapSize(65536);
        EXPECT_EQ(app.jsHeapSize, 65536);
    }

    TEST_F(JsAppTest, SetArgsColorModeTest)
    {
        TestJsApp app;
        app.SetArgsColorMode("dark");
        EXPECT_EQ(app.colorMode, "dark");
        EXPECT_EQ(app.GetColorMode(), "dark");
    }

    TEST_F(JsAppTest, SetArgsAceVersionTest)
    {
        TestJsApp app;
        app.SetArgsAceVersion("ACE_2_0");
        EXPECT_EQ(app.aceVersion, "ACE_2_0");
    }

    TEST_F(JsAppTest, ColorModeChangedTest)
    {
        TestJsApp app;
        app.ColorModeChanged("light");
        EXPECT_EQ(app.GetColorMode(), "light");
    }

    TEST_F(JsAppTest, OrientationChangedTest)
    {
        TestJsApp app;
        app.OrientationChanged("landscape");
        EXPECT_EQ(app.orientation, "landscape");
        EXPECT_EQ(app.GetOrientation(), "landscape");
    }

    TEST_F(JsAppTest, GetJSONTreeDefaultTest)
    {
        TestJsApp app;
        EXPECT_EQ(app.GetJSONTree(), "");
    }

    TEST_F(JsAppTest, GetDefaultJSONTreeDefaultTest)
    {
        TestJsApp app;
        EXPECT_EQ(app.GetDefaultJSONTree(), "");
    }

    TEST_F(JsAppTest, MemoryRefreshDefaultTest)
    {
        TestJsApp app;
        EXPECT_FALSE(app.MemoryRefresh("pages/Index"));
    }

    TEST_F(JsAppTest, NoopVirtualMethodsTest)
    {
        TestJsApp app;
        ResolutionParam param(100, 200, 100, 200);
        Json2::Value val = JsonReader::CreateNull();
        // 以下基类实现均为空实现，调用不应崩溃
        app.ResolutionChanged(param, 480, "resize");
        app.ReloadRuntimePage("pages/Index");
        app.LoadDocument("pages/Index", "name", val);
        app.FoldStatusChanged("fold", 100, 200);
        AvoidAreas areas;
        app.SetAvoidArea(areas);
        app.InitJsApp();
        SUCCEED();
    }

    TEST_F(JsAppTest, GetCurrentAvoidAreaDefaultTest)
    {
        TestJsApp app;
        AvoidAreas areas = app.GetCurrentAvoidArea();
        AvoidAreas expected;
        EXPECT_TRUE(areas == expected);
        EXPECT_EQ(areas.topRect.posX, 0);
        EXPECT_EQ(areas.topRect.posY, 0);
        EXPECT_EQ(areas.topRect.width, 0);
        EXPECT_EQ(areas.topRect.height, 0);
        EXPECT_EQ(areas.leftRect.posX, 0);
        EXPECT_EQ(areas.rightRect.posY, 0);
        EXPECT_EQ(areas.bottomRect.width, 0);
        EXPECT_EQ(areas.bottomRect.height, 0);
    }

    TEST_F(JsAppTest, AvoidRectTest)
    {
        AvoidRect rect(1, 2, 3, 4);
        EXPECT_EQ(rect.posX, 1);
        EXPECT_EQ(rect.posY, 2);
        EXPECT_EQ(rect.width, 3);
        EXPECT_EQ(rect.height, 4);

        AvoidRect copyRect(rect);
        EXPECT_TRUE(copyRect == rect);

        AvoidRect assignedRect;
        assignedRect = rect;
        EXPECT_TRUE(assignedRect == rect);

        AvoidRect diffRect(1, 2, 3, 5);
        EXPECT_FALSE(diffRect == rect);
        EXPECT_TRUE(diffRect == diffRect);
    }

    TEST_F(JsAppTest, AvoidAreasTest)
    {
        AvoidRect top(0, 0, 100, 10);
        AvoidRect left(0, 0, 10, 100);
        AvoidRect right(90, 0, 10, 100);
        AvoidRect bottom(0, 90, 100, 10);
        AvoidAreas areas(top, left, right, bottom);

        AvoidAreas copyAreas(areas);
        EXPECT_TRUE(copyAreas == areas);

        AvoidAreas assignedAreas;
        assignedAreas = areas;
        EXPECT_TRUE(assignedAreas == areas);
        EXPECT_TRUE(assignedAreas.topRect == top);
        EXPECT_TRUE(assignedAreas.leftRect == left);
        EXPECT_TRUE(assignedAreas.rightRect == right);
        EXPECT_TRUE(assignedAreas.bottomRect == bottom);

        AvoidAreas defaultAreas;
        EXPECT_FALSE(defaultAreas == areas);
        EXPECT_TRUE(defaultAreas == AvoidAreas());
    }
}
