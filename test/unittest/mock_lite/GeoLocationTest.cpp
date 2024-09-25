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
#include "location.h"
#include "SharedData.h"
#define private public
#include "VirtualLocation.h"

namespace {
    struct LocSuccessData {
        LocationData data;
    };

    TEST(GeoLocationTest, GetGeoLocationTypeTest)
    {
        EXPECT_EQ(std::string(GetGeoLocationType()), "gps");
    }

    TEST(GeoLocationTest, GetSupportedGeoCoordTypesTest)
    {
        EXPECT_EQ(std::string(GetSupportedGeoCoordTypes()), "wgs84");
    }

    void LocDataUpdateCallback(const int8_t *data, uint32_t len)
    {
        (void)data;
        (void)len;
    }

    TEST(GeoLocationTest, GetGeoLocationTest)
    {
        int32_t ret = GetGeoLocation(nullptr, nullptr, nullptr, 0);
        EXPECT_EQ(ret, -1); // -1 is expect return value
        int32_t ret1 = GetGeoLocation(LocDataUpdateCallback, nullptr, nullptr, 0);
        EXPECT_EQ(ret1, 0); // 0 is expect return value
    }

    TEST(GeoLocationTest, SubGeoLocationTest)
    {
        int32_t ret = SubGeoLocation(LocDataUpdateCallback);
        EXPECT_EQ(ret, 0); // 0 is expect return value
    }

    TEST(GeoLocationTest, UnSubGeoLocationTest)
    {
        UnSubGeoLocation(LocDataUpdateCallback);
        EXPECT_FALSE(VirtualLocation::GetInstance().isSubsribe);
    }

    TEST(GeoLocationTest, DestroyGeoLocationTest)
    {
        DestroyGeoLocation(LocDataUpdateCallback);
        EXPECT_FALSE(VirtualLocation::GetInstance().isSubsribe);
    }

    TEST(GeoLocationTest, SerializeLocDataTest)
    {
        int32_t ret = SerializeLocData(nullptr, 0, nullptr);
        EXPECT_EQ(ret, -1); // -1 is expect return value
        ret = SerializeLocData(VirtualLocation::GetInstance().GetMockPointer(), 0, nullptr);
        EXPECT_EQ(ret, -1); // -1 is expect return value
        uint32_t len = 1;
        ret = SerializeLocData(VirtualLocation::GetInstance().GetMockPointer(), len, nullptr);
        EXPECT_EQ(ret, -1); // -1 is expect return value
        LocationData rspData;
        ret = SerializeLocData(VirtualLocation::GetInstance().GetMockPointer(), len, &rspData);
        EXPECT_EQ(ret, 0); // 0 is expect return value
    }
}