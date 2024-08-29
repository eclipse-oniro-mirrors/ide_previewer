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

#include "VirtualLocation.h"

#include <chrono>
#include <cmath>
#include <cstdlib>

#include "PreviewerEngineLog.h"
#include "SharedData.h"
#include "SharedDataManager.h"

VirtualLocation& VirtualLocation::GetInstance()
{
    static VirtualLocation stance;
    return stance;
}

const int8_t* VirtualLocation::GetMockPointer() const
{
    return mockPointer;
}

uint32_t VirtualLocation::GetMockLen() const
{
    return mockLen;
}

void VirtualLocation::SetCallBack(LocDataUpdateCallback func)
{
    callBack = func;
}

void VirtualLocation::SetSubscribe(bool value)
{
    isSubsribe = value;
}

bool VirtualLocation::IsSubscribe() const
{
    return isSubsribe;
}

LocDataUpdateCallback VirtualLocation::GetCallBack() const
{
    return callBack;
}

void VirtualLocation::ExecCallBack() const
{
    if (callBack != nullptr && isSubsribe) {
        callBack(mockPointer, mockLen);
    }
}

VirtualLocation::VirtualLocation()
    : isSubsribe(false),
      longitudeChecked(0),
      latitudeChecked(0),
      accuracy(0),
      mockPointer(nullptr),
      mockLen(1),
      callBack(nullptr)
{
    accuracy = LOCATION_ACCURACY;
    InitMockPointer();
}

VirtualLocation::~VirtualLocation()
{
    if (mockPointer != nullptr) {
        delete mockPointer;
        mockPointer = nullptr;
    }
}

void VirtualLocation::InitMockPointer()
{
    mockPointer = new(std::nothrow) int8_t;
    if (mockPointer == nullptr) {
        ELOG("Memory allocation failed: mockPointer.");
    }
}

uint64_t VirtualLocation::GetTime() const
{
    time_t curTime = time(nullptr);
    if (curTime < 0) {
        return 0;
    }
    return static_cast<uint64_t>(curTime);
}

float VirtualLocation::GetAccuracy() const
{
    return accuracy;
}

bool VirtualLocation::IsPostionChanged()
{
    double longitude = SharedData<double>::GetData(SharedDataType::LONGITUDE);
    double latitude = SharedData<double>::GetData(SharedDataType::LATITUDE);
    if (std::abs(longitude - longitudeChecked) < pow(PRECISION_BASE_NUMBER, -SharedDataManager::POSITIONPRECISION) &&
        std::abs(latitude - latitudeChecked) < pow(PRECISION_BASE_NUMBER, -SharedDataManager::POSITIONPRECISION)) {
        return false;
    }
    longitudeChecked = longitude;
    latitudeChecked = latitude;
    return true;
}
