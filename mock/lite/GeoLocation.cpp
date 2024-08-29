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

#include "location.h"

#include "PreviewerEngineLog.h"
#include "SharedData.h"
#include "VirtualLocation.h"

static const int32_t RES_OK = 0;
static const int32_t RES_ERROR = -1;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

const char* GetGeoLocationType(void)
{
    return "gps";
}

const char* GetSupportedGeoCoordTypes()
{
    return "wgs84";
}

int32_t GetGeoLocation(LocDataUpdateCallback callback, LocTimeoutCallback timeoutCallback, void *arg, uint32_t timeout)
{
    (void)timeoutCallback;
    (void)arg;
    (void)timeout;
    if (callback == nullptr) {
        ELOG("callback is nullptr");
        return RES_ERROR;
    }
    callback(VirtualLocation::GetInstance().GetMockPointer(), VirtualLocation::GetInstance().GetMockLen());
    return RES_OK;
}

int32_t SubGeoLocation(LocDataUpdateCallback callback)
{
    callback(VirtualLocation::GetInstance().GetMockPointer(), VirtualLocation::GetInstance().GetMockLen());
    VirtualLocation::GetInstance().SetCallBack(callback);
    VirtualLocation::GetInstance().SetSubscribe(true);
    return RES_OK;
}

void UnSubGeoLocation(LocDataUpdateCallback callback)
{
    if (callback == VirtualLocation::GetInstance().GetCallBack()) {
        VirtualLocation::GetInstance().SetSubscribe(false);
    }
}

void DestroyGeoLocation(LocDataUpdateCallback callback)
{
    VirtualLocation::GetInstance().SetCallBack(callback);
    VirtualLocation::GetInstance().SetSubscribe(false);
}

int32_t SerializeLocData(const int8_t *rawData, uint32_t len, LocationData *result)
{
    if (rawData != VirtualLocation::GetInstance().GetMockPointer()) {
        ELOG("Serialize rawData invalid.");
        return RES_ERROR;
    }

    if (len != VirtualLocation::GetInstance().GetMockLen()) {
        ELOG("Serialize len invalid.");
        return RES_ERROR;
    }

    if (result == nullptr) {
        ELOG("result is nullptr.");
        return RES_ERROR;
    }

    result->accuracy = VirtualLocation::GetInstance().GetAccuracy();
    result->latitude = SharedData<double>::GetData(SharedDataType::LATITUDE);
    result->longitude = SharedData<double>::GetData(SharedDataType::LONGITUDE);
    result->time = VirtualLocation::GetInstance().GetTime();
    return RES_OK;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* __cplusplus */
