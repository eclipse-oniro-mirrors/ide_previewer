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
#include "gtest/gtest.h"
#include "CrashHandler.h"

namespace {
    class CrashHandlerTest : public ::testing::Test {
    protected:
        static void SetUpTestCase()
        {
            // 在测试开始前重置信号处理函数为空
            if (signal(SIGSEGV, SIG_DFL) == SIG_ERR) {
                printf("set signal failed");
            }
        }

        static void TearDownTestCase()
        {
            // 在测试结束后重置信号处理函数为空
            if (signal(SIGSEGV, SIG_DFL) == SIG_ERR) {
                printf("set signal failed");
            }
        }
    };

    TEST_F(CrashHandlerTest, InitExceptionHandlerTest)
    {
        // 保存原始的信号处理函数
        void (*originalHandler)(int) = signal(SIGSEGV, SIG_DFL);
        // init exception handler
        auto richCrashHandler = std::make_unique<CrashHandler>();
        richCrashHandler->InitExceptionHandler();
        // 检查信号处理函数是否被正确注册
        EXPECT_NE(signal(SIGSEGV, SIG_DFL), originalHandler);
    }
}