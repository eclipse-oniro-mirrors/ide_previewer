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

#include "CppTimer.h"

CppTimer::~CppTimer()
{
    std::thread::id curThreadId = std::this_thread::get_id();
    if (curThreadId != threadId) {
        ILOG("CppTimer can not deleted by other thread!");
    }
}

void CppTimer::Start(int64_t value)
{
    std::thread::id curThreadId = std::this_thread::get_id();
    if (curThreadId != threadId) {
        ILOG("CppTimer can not started by other thread!");
    }

    this->interval = value;
    startTime = std::chrono::system_clock::now();
    isRunning = true;
}

void CppTimer::Stop()
{
    std::thread::id curThreadId = std::this_thread::get_id();
    if (curThreadId != threadId) {
        ILOG("CppTimer can not stoped by other thread!");
    }
    isRunning = false;
}

void CppTimer::RunTimerTick(CallbackQueue& queue)
{
    if (interval == 0) {
        return;
    }

    if (!isRunning) {
        return;
    }

    std::thread::id curThreadId = std::this_thread::get_id();
    auto endTime = std::chrono::system_clock::now();

    if (curThreadId != threadId) {
        ILOG("CppTimer can not run in other thread");
        return;
    }

    int64_t timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    if (timePassed < interval) {
        return;
    }

    if (shotTimes != 0) {
        queue.AddCallback(functional);
        startTime = endTime;
    }

    if (shotTimes > 0) {
        shotTimes--;
    }
}
