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
#include "sensor_impl.h"
#include "SharedData.h"

namespace {
    uint32_t g_pressureVal = 1000;
    uint32_t g_sumStepVal = 2000;
    uint8_t g_heartBeatVal = 90;
    bool g_wearingStateVal = true;

    class SensorModuleImplTest : public ::testing::Test {
    public:
        SensorModuleImplTest() {}
        ~SensorModuleImplTest() {}
    protected:
        static void SetUpTestCase()
        {
            SharedData<uint32_t>(SharedDataType::PRESSURE_VALUE, g_pressureVal, 0, 999900); // 999900 is max value
            SharedData<uint32_t>(SharedDataType::SUMSTEP_VALUE, g_sumStepVal, 0, 999999); // 999999 is max value
            SharedData<uint8_t>(SharedDataType::HEARTBEAT_VALUE, g_heartBeatVal, 0, 255); // 255 is max value
            SharedData<bool>(SharedDataType::WEARING_STATE, g_wearingStateVal);
        }
    };

    TEST_F(SensorModuleImplTest, GetBarometerTest)
    {
        uint32_t ret = OHOS::ACELite::SensorImpl::GetBarometer();
        EXPECT_EQ(ret, g_pressureVal);
    }

    TEST_F(SensorModuleImplTest, GetStepsTest)
    {
        uint32_t ret = OHOS::ACELite::SensorImpl::GetSteps();
        EXPECT_EQ(ret, g_sumStepVal);
    }

    TEST_F(SensorModuleImplTest, GetHeartRateTest)
    {
        uint32_t ret = OHOS::ACELite::SensorImpl::GetHeartRate();
        EXPECT_EQ(ret, g_heartBeatVal);
    }

    TEST_F(SensorModuleImplTest, GetOnBodyStateTest)
    {
        bool ret = OHOS::ACELite::SensorImpl::GetOnBodyState();
        EXPECT_EQ(ret, g_wearingStateVal);
    }
}