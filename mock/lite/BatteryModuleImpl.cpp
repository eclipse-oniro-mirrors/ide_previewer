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


#include "battery_impl.h"

#include "PreviewerEngineLog.h"
#include "SharedData.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t GetBatteryStatus(bool *charging, double *level)
{
    if (charging == nullptr) {
        ILOG("charging is nullptr");
        return -1;
    }
    if (level == nullptr) {
        ILOG("level is nullptr");
        return -1;
    }
    *level = SharedData<double>::GetData(SharedDataType::BATTERY_LEVEL);
    *charging = SharedData<uint8_t>::GetData(SharedDataType::BATTERY_STATUS);
    ILOG("Battery charge state: %d level: %f", *charging, *level);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* __cplusplus */
