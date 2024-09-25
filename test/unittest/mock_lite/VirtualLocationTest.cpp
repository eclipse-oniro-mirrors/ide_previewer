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
#include "VirtualLocation.h"
#include "SharedData.h"

namespace {
    bool g_executeCallback = false;

    void LocDataUpdateCallback(const int8_t *data, uint32_t len)
    {
        (void)data;
        (void)len;
        g_executeCallback = true;
    }

    TEST(VirtualLocationTest, GetMockPointerTest)
    {
        const int8_t* ptr = VirtualLocation::GetInstance().GetMockPointer();
        EXPECT_TRUE(ptr != nullptr);
    }

    TEST(VitualLocationTest, GetMockLenTest)
    {
        uint32_t len = VirtualLocation::GetInstance().GetMockLen();
        EXPECT_EQ(len, 1); // 1 is default mockLen value
    }

    TEST(VirtualLocationTest, SetCallBackTest)
    {
        VirtualLocation::GetInstance().SetCallBack(LocDataUpdateCallback);
        EXPECT_NE(VirtualLocation::GetInstance().GetCallBack(), nullptr);
    }

    TEST(VirtualLocationTest, SetSubscribeTest)
    {
        VirtualLocation::GetInstance().SetSubscribe(true);
        EXPECT_TRUE(VirtualLocation::GetInstance().IsSubscribe());
    }

    TEST(VirtualLocationTest, ExecCallBackTest)
    {
        VirtualLocation::GetInstance().ExecCallBack();
        EXPECT_TRUE(g_executeCallback);
    }

    TEST(VirtualLocationTest, GetTimeTest)
    {
        uint64_t ret = VirtualLocation::GetInstance().GetTime();
        EXPECT_TRUE(ret > 0);
    }

    TEST(VirtualLocationTest, GetAccuracyTest)
    {
        float ret = VirtualLocation::GetInstance().GetAccuracy();
        EXPECT_EQ(ret, VirtualLocation::GetInstance().LOCATION_ACCURACY);
    }

    TEST(VirtualLocationTest, IsPositionChangedTest)
    {
        SharedData<double>(SharedDataType::LONGITUDE, 0, -180, 180);
        SharedData<double>(SharedDataType::LATITUDE, 0, -90, 90);

        VirtualLocation::GetInstance().longitudeChecked = 0.0;
        VirtualLocation::GetInstance().latitudeChecked = 0.0;
        SharedData<double>::SetData(SharedDataType::LONGITUDE, 0.0);
        SharedData<double>::SetData(SharedDataType::LATITUDE, 0.0);
        bool ret1 = VirtualLocation::GetInstance().IsPostionChanged();
        EXPECT_FALSE(ret1);

        SharedData<double>::SetData(SharedDataType::LONGITUDE, 50.0);
        SharedData<double>::SetData(SharedDataType::LATITUDE, 50.0);
        bool ret2 = VirtualLocation::GetInstance().IsPostionChanged();
        EXPECT_TRUE(ret2);
    }
}