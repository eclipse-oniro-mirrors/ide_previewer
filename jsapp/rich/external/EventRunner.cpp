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

#include "EventRunner.h"

namespace OHOS::Ide {
EventRunner& EventRunner::Current()
{
    static EventRunner mainRunner;
    return mainRunner;
}

EventRunner& EventRunner::GetMainEventRunner()
{
    return Current();
}

void EventRunner::SetMainThreadId(std::thread::id id)
{
    threadId = id;
}

std::thread::id EventRunner::GetThreadId()
{
    return threadId;
}

bool EventRunner::IsCurrentRunnerThread()
{
    return std::this_thread::get_id() == threadId;
}

void EventRunner::Run()
{
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::vector<Callback> copyTasks;
    // Process expired tasks.
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (!queue.empty()) {
            const auto& top = queue.top();
            // If the task at the top of task queue has not yet expired, there is nothing more to do.
            if (top.GetTargetTime() > now) {
                break;
            }
            // Only record tasks without executing them when the task queue mutex is hold.
            copyTasks.push_back(top.GetTask());
            queue.pop();
        }
    }
    {
        // Flushing tasks here without holing onto the task queue mutex.
        for (const auto& task : copyTasks) {
            task();
        }
    }
}

void EventRunner::PushTask(const Callback &callback, std::chrono::steady_clock::time_point targetTime)
{
    static size_t order = 0;
    std::lock_guard<std::mutex> lock(mutex);
    order++;
    queue.push({ order, callback, targetTime });
}
}