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
#define private public
#include "EventHandler.h"

namespace {
    int g_originNum = 0;
    int g_replacedNum = 3;

    // 测试拷贝构造函数是否被删除
    TEST(EventHandlerTest, CopyConstructorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_constructible<OHOS::AppExecFwk::EventHandler>::value == false);
    }

    // 测试赋值运算符是否被删除
    TEST(EventHandlerTest, AssignmentOperatorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_assignable<OHOS::AppExecFwk::EventHandler>::value == false);
    }

    TEST(EventHandlerTest, SetMainThreadIdTest)
    {
        std::thread::id tid = std::this_thread::get_id();
        OHOS::AppExecFwk::EventHandler::SetMainThreadId(tid);
        EXPECT_EQ(OHOS::AppExecFwk::EventRunner::GetMainEventRunner().threadId, tid);
    }

    TEST(EventHandlerTest, IsCurrentRunnerThreadTest)
    {
        std::thread commandThead([]() {
            EXPECT_FALSE(OHOS::AppExecFwk::EventHandler::IsCurrentRunnerThread());
        });
        commandThead.join();
        EXPECT_TRUE(OHOS::AppExecFwk::EventHandler::IsCurrentRunnerThread());
    }

    TEST(EventHandlerTest, PostTaskTest)
    {
        OHOS::AppExecFwk::EventHandler::PostTask([]() {
            g_originNum = g_replacedNum;
        });
        EXPECT_TRUE(OHOS::AppExecFwk::EventRunner::GetMainEventRunner().queue.size() > 0);
    }

    TEST(EventHandlerTest, RunTest)
    {
        OHOS::AppExecFwk::EventHandler::Run();
        EXPECT_EQ(g_originNum, g_replacedNum);
    }
}