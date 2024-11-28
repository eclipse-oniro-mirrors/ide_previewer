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

#include "CppTimerManager.h"

#include <thread>

std::map<std::thread::id, std::unique_ptr<CppTimerManager>> CppTimerManager::managers;

// Non-threaded security
CppTimerManager& CppTimerManager::GetTimerManager()
{
    std::thread::id curThreadId = std::this_thread::get_id();
    if (managers.find(curThreadId) == managers.end()) {
        managers[curThreadId] = std::make_unique<CppTimerManager>();
    }
    return *managers[curThreadId];
}

void CppTimerManager::AddCppTimer(CppTimer& timer)
{
    runningTimers.push_back(&timer);
    ILOG("CppTimerManager::AddCppTimer");
}

void CppTimerManager::RemoveCppTimer(CppTimer& timer)
{
    runningTimers.remove(&timer);
    ILOG("CppTimerManager::RemoveCppTimer");
}

void CppTimerManager::RunTimerTick()
{
    std::list<CppTimer*> tempTimers = runningTimers;
    if (tempTimers.size() == 0) {
        ILOG("CppTimerManager::RunTimerTick No timer exec.");
    }
    auto iter = tempTimers.cbegin();
    while (iter != tempTimers.cend()) {
        CppTimer* timer = *iter;
        timer->RunTimerTick(callbackQueue);

        iter++;
    }

    callbackQueue.ConsumingCallback();
}
