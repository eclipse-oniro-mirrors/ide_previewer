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
#include <thread>
#include "gtest/gtest.h"
#define private public
#include "CallbackQueue.h"
using namespace std;

namespace {
    TEST(CallbackQueueTest, AddCallbackTest)
    {
        CallbackQueue queue;
        bool callbackCalled = false;

        // 添加回调函数到队列中
        queue.AddCallback([&callbackCalled]() {
            callbackCalled = true;
        });

        // 执行队列中的回调函数
        queue.ConsumingCallback();

        // 验证回调函数是否被调用
        EXPECT_TRUE(callbackCalled);
    }

    TEST(CallbackQueueTest, ConsumingCallbackTest)
    {
        CallbackQueue queue;

        // 添加多个回调函数到队列中
        for (int i = 0; i < 5; ++i) {
            queue.AddCallback([]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟耗时操作
            });
        }

        // 启动多个线程同时执行 ConsumingCallback() 方法
        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back([&queue]() {
                queue.ConsumingCallback();
            });
        }

        // 等待所有线程执行完成
        for (auto& thread : threads) {
            thread.join();
        }

        // 验证队列中的回调函数是否都被执行完毕
        EXPECT_EQ(queue.callBackList.size(), 0);
    }
}