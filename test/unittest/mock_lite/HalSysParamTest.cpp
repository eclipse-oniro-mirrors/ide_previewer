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
#include "hal_sys_param.h"
#include "FileSystem.h"
#include "ModelManager.h"

namespace {
    class HalSysParamTest : public ::testing::Test {
    public:
        HalSysParamTest() {}
        ~HalSysParamTest() {}
    protected:
        static void SetUpTestCase()
        {
            ModelManager::SetCurrentDevice("liteWearable");
        }
    };

    TEST_F(HalSysParamTest, HalGetDeviceTypeTest)
    {
        const char* str = HalGetDeviceType();
        EXPECT_EQ(std::string(str), "liteWearable");
    }

    TEST_F(HalSysParamTest, HalGetManufactureTest)
    {
        const char* str = HalGetManufacture();
        EXPECT_EQ(std::string(str), "***");
    }

    TEST_F(HalSysParamTest, HalGetBrandTest)
    {
        const char* str = HalGetBrand();
        EXPECT_EQ(std::string(str), "***");
    }

    TEST_F(HalSysParamTest, HalGetMarketNameTest)
    {
        const char* str = HalGetMarketName();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetProductSeriesTest)
    {
        const char* str = HalGetProductSeries();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetProductModelTest)
    {
        const char* str = HalGetProductModel();
        EXPECT_EQ(std::string(str), "***");
    }

    TEST_F(HalSysParamTest, HalGetSoftwareModelTest)
    {
        const char* str = HalGetSoftwareModel();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetHardwareModelTest)
    {
        const char* str = HalGetHardwareModel();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetHardwareProfileTest)
    {
        const char* str = HalGetHardwareProfile();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetSerialTest)
    {
        const char* str = HalGetSerial();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetBootloaderVersionTest)
    {
        const char* str = HalGetBootloaderVersion();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetAbiListTest)
    {
        const char* str = HalGetAbiList();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetDisplayVersionTest)
    {
        const char* str = HalGetDisplayVersion();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetIncrementalVersionTest)
    {
        const char* str = HalGetIncrementalVersion();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetBuildTypeTest)
    {
        const char* str = HalGetBuildType();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetBuildUserTest)
    {
        const char* str = HalGetBuildUser();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetBuildHostTest)
    {
        const char* str = HalGetBuildHost();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetBuildTimeTest)
    {
        const char* str = HalGetBuildTime();
        EXPECT_EQ(std::string(str), "");
    }

    TEST_F(HalSysParamTest, HalGetFirstApiVersionTest)
    {
        EXPECT_EQ(HalGetFirstApiVersion(), 0);
    }
}