/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "battery_impl.h"
#include "SharedData.h"

namespace {
    TEST(BatteryModuleImplTest, GetBatteryStatusTest)
    {
        double minLvl = 0.0;
        double curLvl = 0.5;
        double maxLvl = 1.0;
        SharedData<uint8_t>(SharedDataType::BATTERY_STATUS, (uint8_t)ChargeState::NOCHARGE,
            (uint8_t)ChargeState::NOCHARGE, (uint8_t)ChargeState::CHARGING);
        SharedData<double>(SharedDataType::BATTERY_LEVEL, curLvl, minLvl, maxLvl);
        bool *charging = nullptr;
        double *level = nullptr;
        int32_t ret = GetBatteryStatus(charging, level);
        EXPECT_EQ(ret, -1); // -1 is function return value

        bool testCharging = true;
        double testLevel = 0.0;
        int32_t ret1 = GetBatteryStatus(&testCharging, level);
        EXPECT_EQ(ret1, -1); // -1 is function return value

        int32_t ret2 = GetBatteryStatus(&testCharging, &testLevel);
        EXPECT_EQ(ret2, 0); // 0 is function return value
        EXPECT_FALSE(testCharging);
        EXPECT_EQ(testLevel, curLvl);
    }
}