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

#include "nativeapi_timer_task.h"

#include "CppTimer.h"
#include "CppTimerManager.h"
#include "PreviewerEngineLog.h"

const static int RES_OK = 0;
const static int RES_ERROR = -1;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int InitTimerTask()
{
    return RES_OK;
}

using TimerCallBack = void(*)(void*);

int StartTimerTask(bool isPeriodic, const unsigned int delay, void* userCallback,
                   void* userContext, timerHandle_t *timerHandle)
{
    if (timerHandle == nullptr) {
        ELOG("timerHandle is nullptr");
        return RES_ERROR;
    }

    CppTimer* timer = new(std::nothrow) CppTimer(reinterpret_cast<TimerCallBack>(userCallback), userContext);
    if (timer == nullptr) {
        ELOG("Memory allocation failed: timer.");
        *timerHandle = timer;
        return RES_ERROR;
    }

    if (!isPeriodic) {
        timer->SetShotTimes(1);
    }
    timer->Start(delay);
    CppTimerManager::GetTimerManager().AddCppTimer(*timer);
    *timerHandle = timer;
    return RES_OK;
}

int StopTimerTask(const timerHandle_t timerHandle)
{
    if (timerHandle == nullptr) {
        ELOG("TimerTaskHandler::StartTimer timerHandle is null.");
        return RES_ERROR;
    }

    CppTimer* timer = static_cast<CppTimer*>(timerHandle);
    if (timer == nullptr) {
        ELOG("TimerTaskHandler::StartTimer timerHandle is invalud.");
        return RES_ERROR;
    }
    timer->Stop();
    CppTimerManager::GetTimerManager().RemoveCppTimer(*timer);
    delete timer;
    timer = nullptr;
    return RES_OK;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* __cplusplus */
