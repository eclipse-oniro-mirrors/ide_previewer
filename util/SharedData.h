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

#ifndef SHAREDDATA_H
#define SHAREDDATA_H

#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <thread>

#include "SharedDataManager.h"
#include "PreviewerEngineLog.h"

template<typename T> class SharedData {
public:
    SharedData() : value(), isChanged(false), minValue(), maxValue() {}

    ~SharedData() {}

    SharedData(SharedDataType type, T v, T min = T(), T max = T()) : value(v), minValue(min), maxValue(max)
    {
        staticDataMutex.lock();
        isChanged = false;
        dataMap[type] = *this;
        staticDataMutex.unlock();

        SharedDataManager::AddChecker(SharedData<T>::CheckAll);
    }

    static bool SetData(SharedDataType type, T v)
    {
        staticDataMutex.lock();
        if (dataMap.find(type) == dataMap.end()) {
            staticDataMutex.unlock();
            FLOG("SharedData::SetData invalid data type.");
            return false;
        }

        SharedData data = dataMap[type];
        if ((data.minValue != data.maxValue) && (v < data.minValue || v > data.maxValue)) {
            staticDataMutex.unlock();
            return false;
        }
        data.value = v;
        data.isChanged = true;
        dataMap[type] = data;
        staticDataMutex.unlock();
        return true;
    }

    static T GetData(SharedDataType type)
    {
        const std::lock_guard<std::mutex> lock(staticDataMutex);
        if (dataMap.find(type) == dataMap.end()) {
            FLOG("SharedData::GetData invalid data type.");
        }

        return dataMap[type].value;
    }

    static bool IsValid(SharedDataType type, T v)
    {
        const std::lock_guard<std::mutex> lock(staticDataMutex);
        if (dataMap.find(type) == dataMap.end()) {
            FLOG("SharedData::IsValid invalid data type.");
        }

        if ((dataMap[type].minValue != dataMap[type].maxValue) &&
            (v < dataMap[type].minValue || v > dataMap[type].maxValue)) {
            ILOG("%d %d %d", v, dataMap[type].minValue, dataMap[type].maxValue);
            return false;
        }

        return true;
    }

    /*
     * Checks whether the current data is changed.
     * Return a list of data changed callback
     */
    std::list<std::pair<std::function<void(T)>, T>> Check()
    {
        std::thread::id curThreadId = std::this_thread::get_id();
        if (ticksMap.find(curThreadId) == ticksMap.end()) {
            ticksMap[curThreadId] = 0;
        }
        uint32_t ticks = ticksMap[curThreadId] + 1;
        ticksMap[curThreadId] = ticks;

        std::list<std::pair<std::function<void(T)>, T>> pairs;
        if (!isChanged) {
            return pairs;
        }
        for (auto iter = callBacks.begin(); iter != callBacks.end(); ++iter) {
            if (iter->first == curThreadId && ticks % iter->second.second == 0) {
                isChanged = false;
                pairs.push_back(std::make_pair(iter->second.first, value));
            }
        }

        return pairs;
    }

    /*
     * Add a data changed callback
     * type: Checked data type
     * func: Callback
     * threadId: Checks and callbacks are performed only in this thread.
     * period: Indicates the detection period. The unit is 100 ms.
     */
    static void
        AppendNotify(SharedDataType type, std::function<void(T)> func, std::thread::id threadId, uint32_t period = 1)
    {
        staticDataMutex.lock();
        if (dataMap.find(type) == dataMap.end()) {
            FLOG("SharedData::IsValid invalid data type.");
        }
        dataMap[type].callBacks[threadId] = std::pair<std::function<void(T)>, uint32_t>(func, period);
        staticDataMutex.unlock();
    }

    // Check whether the data is changes and execute the notification callback.
    static void CheckAll()
    {
        std::list<std::pair<std::function<void(T)>, T>> pairs;
        staticDataMutex.lock();
        for (auto dataIter = dataMap.begin(); dataIter != dataMap.end(); ++dataIter) {
            auto checkers = dataIter->second.Check();
            pairs.insert(pairs.end(), checkers.begin(), checkers.end());
        }
        staticDataMutex.unlock();

        for (auto pair : pairs) {
            pair.first(pair.second);
        }
    }

private:
    T value;
    bool isChanged;
    T minValue;
    T maxValue;
    // map<thread id, pair<callback, detection period>>
    std::map<std::thread::id, std::pair<std::function<void(T)>, uint32_t>> callBacks;
    // map<thread id, current time tick>
    std::map<std::thread::id, uint32_t> ticksMap;
    static std::map<SharedDataType, SharedData> dataMap;
    static std::mutex staticDataMutex;
};

template<typename T> std::map<SharedDataType, SharedData<T>> SharedData<T>::dataMap;

template<typename T> std::mutex SharedData<T>::staticDataMutex;

#endif // SHAREDDATA_H
