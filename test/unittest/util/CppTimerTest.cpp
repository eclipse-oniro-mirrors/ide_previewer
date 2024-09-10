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
#include "CppTimer.h"
#include "CallbackQueue.h"

namespace {
    static void Double(int& addNum)
    {
        addNum += addNum;
    }
    
    // 测试默认构造函数是否被删除
    TEST(CppTimerTest, DefaultConstructorDeletedTest)
    {
        EXPECT_TRUE(std::is_default_constructible<CppTimer>::value == false);
    }

    // 测试拷贝构造函数是否被删除
    TEST(CppTimerTest, CopyConstructorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_constructible<CppTimer>::value == false);
    }

    // 测试赋值运算符是否被删除
    TEST(CppTimerTest, AssignmentOperatorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_assignable<CppTimer>::value == false);
    }
    
    TEST(CppTimerTest, SetShotTimesTest)
    {
        // same thread
        int addNum = 3;
        CppTimer timer(Double, std::ref(addNum));
        int newShotTime = 3;
        timer.SetShotTimes(newShotTime);
        EXPECT_EQ(timer.GetShotTimes(), newShotTime);
        // not same thread
        std::thread commandThead([&timer, &newShotTime]() {
            timer.SetShotTimes(newShotTime);
            EXPECT_EQ(timer.GetShotTimes(), newShotTime);
        });
        commandThead.detach();
    }

    TEST(CppTimerTest, IsRunningTest)
    {
        int addNum = 3;
        int sum = addNum + addNum;
        CppTimer timer(Double, std::ref(addNum));
        CallbackQueue queue;
        int interval = 100;
        // normal
        timer.Start(interval);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        EXPECT_EQ(timer.interval, interval);
        EXPECT_TRUE(timer.IsRunning());
        timer.RunTimerTick(queue);
        EXPECT_TRUE(queue.callBackList.size() > 0);
        queue.ConsumingCallback();
        EXPECT_EQ(addNum, sum);
        timer.Stop();
        EXPECT_FALSE(timer.IsRunning());
    }

    TEST(CppTimerTest, IsRunningTest2)
    {
        int addNum = 3;
        CppTimer timer(Double, std::ref(addNum));
        CallbackQueue queue;
        int interval = 100;
        // not same thread
        std::thread commandThead([&timer, &queue, &interval]() {
            timer.Start(interval);
            timer.RunTimerTick(queue);
            timer.Stop();
            EXPECT_FALSE(timer.IsRunning());
        });
        commandThead.detach();
    }

    TEST(CppTimerTest, IsRunningTest3)
    {
        int addNum = 3;
        CppTimer timer(Double, std::ref(addNum));
        CallbackQueue queue;
        int interval = 0;
        // interval is 0
        timer.Start(interval);
        timer.RunTimerTick(queue);
        timer.Stop();
        EXPECT_FALSE(timer.IsRunning());
    }
}