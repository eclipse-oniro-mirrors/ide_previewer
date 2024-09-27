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
#define private public
#include "brightness_impl.h"
#include "SharedData.h"

namespace {
    TEST(BrightnessImplTest, SetAndGetValueImplTest)
    {
        uint8_t minVal = 0;
        uint8_t curVal = 100;
        uint8_t maxVal = 100;
        SharedData<uint8_t>(SharedDataType::BRIGHTNESS_VALUE, curVal, minVal, maxVal);
        // set value failed
        uint8_t setVal = 200;
        int32_t ret = OHOS::ACELite::BrightnessImpl::SetValueImpl(setVal);
        EXPECT_EQ(ret, OHOS::ACELite::EC_API_INVALID_PARAM);
        // set value successed
        setVal = 80;
        ret = OHOS::ACELite::BrightnessImpl::SetValueImpl(setVal);
        EXPECT_EQ(ret, 0);
        uint8_t val;
        OHOS::ACELite::BrightnessImpl::GetValueImpl(val);
        EXPECT_EQ(val, setVal);
    }

    TEST(BrightnessImplTest, SetAndGetModeImplTest)
    {
        SharedData<uint8_t>(SharedDataType::BRIGHTNESS_MODE, (uint8_t)BrightnessMode::MANUAL,
            (uint8_t)BrightnessMode::MANUAL, (uint8_t)BrightnessMode::AUTO);
        // set value failed
        uint8_t setVal = 200;
        int32_t ret = OHOS::ACELite::BrightnessImpl::SetModeImpl(setVal);
        EXPECT_EQ(ret, OHOS::ACELite::EC_API_INVALID_PARAM);
        // set value successed
        setVal = 1;
        ret = OHOS::ACELite::BrightnessImpl::SetModeImpl(setVal);
        EXPECT_EQ(ret, 0);
        uint8_t val;
        OHOS::ACELite::BrightnessImpl::GetModeImpl(val);
        EXPECT_EQ(val, setVal);
    }

    TEST(BrightnessImplTest, SetKeepScreenOnImplTest)
    {
        SharedData<bool>(SharedDataType::KEEP_SCREEN_ON, true);
        OHOS::ACELite::BrightnessImpl brightness;
        brightness.SetKeepScreenOnImpl(false);
        EXPECT_FALSE(SharedData<bool>::GetData(SharedDataType::KEEP_SCREEN_ON));
    }

    TEST(BrightnessImplTest, SetAlwaysOnModeTest)
    {
        bool mode = 0;
        OHOS::ACELite::BrightnessImpl::SetAlwaysOnMode(mode);
        EXPECT_EQ(mode, 0);
    }

    TEST(BrightnessImplTest, GetAndSetSysModeTest)
    {
        uint8_t mode = 0;
        OHOS::ACELite::BrightnessImpl::SetSysMode(0);
        int32_t ret = OHOS::ACELite::BrightnessImpl::GetSysMode(mode);
        EXPECT_EQ(ret, 0);
        EXPECT_EQ(mode, 0);
    }

    TEST(BrightnessImplTest, SetAndGetSysAlwaysOnStateImplTest)
    {
        OHOS::ACELite::BrightnessImpl brightness;
        uint8_t alwaysOnState = 1;
        brightness.SetSysAlwaysOnState(alwaysOnState);
        uint8_t ret = brightness.GetSysAlwaysOnState();
        EXPECT_EQ(ret, 0);
    }

    TEST(BrightnessImplTest, GetCurBrightLevelTest)
    {
        uint8_t ret = OHOS::ACELite::BrightnessImpl::GetCurBrightLevel();
        EXPECT_EQ(ret, 0);
    }
}