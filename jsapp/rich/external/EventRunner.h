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

#ifndef EVENT_RUNNER_H
#define EVENT_RUNNER_H

#include <memory>
#include <mutex>
#include <thread>
#include "EventQueue.h"

namespace OHOS::Ide {
class EventRunner final {
public:
    EventRunner() = default;
    ~EventRunner() = default;
    static EventRunner& Current();
    static EventRunner& GetMainEventRunner();
    void SetMainThreadId(std::thread::id id);
    std::thread::id GetThreadId();
    bool IsCurrentRunnerThread();
    void Run();

    void PushTask(const Callback &callback, std::chrono::steady_clock::time_point targetTime);

private:
    EventRunner(const EventRunner&) = delete;
    EventRunner &operator=(const EventRunner&) = delete;
    std::thread::id threadId;
    EventQueue queue;
    std::mutex mutex;
};
}
#endif // EVENT_RUNNER_H
