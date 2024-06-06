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
#include "CppTimerManager.h"
using namespace std;

namespace {
    static void Double(int& addNum)
    {
        addNum += addNum;
    }
    
    // 测试赋值运算符是否被删除
    TEST(CppTimerManagerTest, AssignmentOperatorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_assignable<CppTimerManager>::value == false);
    }

    TEST(CppTimerManagerTest, AddAndRemoveCppTimerTest) {
        int num = 3;
        CppTimerManager& manager = CppTimerManager::GetTimerManager();
        CppTimer timer([&num]() { num += num; });
        int value = 1;
        manager.AddCppTimer(timer);
        EXPECT_EQ(manager.runningTimers.size(), value);
        int interval = 100;
        timer.Start(interval);
        value = num + num;
        this_thread::sleep_for(chrono::milliseconds(200));
        manager.RunTimerTick();
        EXPECT_EQ(num, value);
        value = 0;
        manager.RemoveCppTimer(timer);
        EXPECT_EQ(manager.runningTimers.size(), value);
    }
}