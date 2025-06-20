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

#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <iostream>
#include <queue>
#include <chrono>
#include <functional>

namespace OHOS::Ide {
using Callback = std::function<void()>;

class EventTask {
public:
    EventTask(size_t order, Callback task, std::chrono::steady_clock::time_point targetTime);
    EventTask(const EventTask& other);
    EventTask& operator=(const EventTask& other);
    ~EventTask();
    const Callback& GetTask() const;
    std::chrono::steady_clock::time_point GetTargetTime() const;
    bool operator>(const EventTask& other) const;

private:
    size_t order;
    Callback task;
    std::chrono::steady_clock::time_point targetTime;
};

using EventQueue = std::priority_queue<EventTask, std::deque<EventTask>, std::greater<EventTask>>;
}
#endif // EVENT_QUEUE_H