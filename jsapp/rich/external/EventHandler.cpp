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

#include "EventHandler.h"

namespace OHOS::Ide {
    void EventHandler::SetMainThreadId(std::thread::id id)
    {
        EventRunner::Current().SetMainThreadId(id);
    }

    bool EventHandler::IsCurrentRunnerThread()
    {
        return EventRunner::Current().IsCurrentRunnerThread();
    }

    bool EventHandler::PostTask(const Callback &callback, int64_t delayTime)
    {
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point actualTimePoint = now + std::chrono::milliseconds(delayTime);
        EventRunner::Current().PushTask(callback, actualTimePoint);
        return true;
    }

    void EventHandler::Run()
    {
        EventRunner::Current().Run();
    }
}