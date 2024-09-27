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

#include "gtest/gtest.h"
#include "TimerTaskHandler.h"
#include "AsyncWorkManager.h"
#include "JsAppImpl.h"
#include "MockGlobalResult.h"
#include "CommandLineInterface.h"

namespace {
    TEST(TimerTaskHandlerTest, TaskHandleTest)
    {
        g_taskHandler = false;
        TimerTaskHandler::TaskHandle();
        EXPECT_TRUE(g_taskHandler);
    }

    TEST(TimerTaskHandlerTest, CheckDeviceTest)
    {
        bool value = false;
        OHOS::ACELite::AsyncWorkHandler workHandler = [](void* data) {
            bool* boolData = static_cast<bool*>(data);
            *boolData = true;
        };
        AsyncWorkManager::GetInstance().AppendAsyncWork(workHandler, &value);
        TimerTaskHandler::CheckDevice();
        EXPECT_TRUE(value);
    }

    TEST(TimerTaskHandlerTest, CheckLanguageChangedTest)
    {
        g_globalConfigLanguage = false;
        TimerTaskHandler::CheckLanguageChanged("zh-CN");
        EXPECT_TRUE(g_globalConfigLanguage);
    }

    TEST(TimerTaskHandlerTest, CheckBrightnessValueChangedTest)
    {
        CommandLineInterface::GetInstance().Init("pipeName");
        g_output = false;
        TimerTaskHandler::CheckBrightnessValueChanged(0);
        EXPECT_TRUE(g_output);
    }

    TEST(TimerTaskHandlerTest, CheckJsRunningTest)
    {
        JsAppImpl::GetInstance().SetRunning(false);
        TimerTaskHandler::CheckJsRunning();
        EXPECT_TRUE(JsAppImpl::GetInstance().GetRunning());
    }
}