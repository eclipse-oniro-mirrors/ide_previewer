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
#include "SharedData.h"
#include "SharedDataManager.h"

namespace {
    int g_intValue = 80;
    int g_intMinValue = 0;
    int g_intMaxValue = 255;
    std::string g_strValue = "zh_CN";

    TEST(SharedDataTest, ConstructorTest)
    {
        SharedData<uint8_t>(SharedDataType::HEARTBEAT_VALUE, g_intValue, g_intMinValue, g_intMaxValue);
        EXPECT_EQ(SharedData<uint8_t>::GetData(SharedDataType::HEARTBEAT_VALUE), g_intValue);
        EXPECT_TRUE(SharedData<uint8_t>::IsValid(SharedDataType::HEARTBEAT_VALUE, g_intMinValue));
        EXPECT_TRUE(SharedData<uint8_t>::IsValid(SharedDataType::HEARTBEAT_VALUE, g_intMaxValue));
        SharedData<std::string>(SharedDataType::LANGUAGE, g_strValue);
        EXPECT_EQ(SharedData<std::string>::GetData(SharedDataType::LANGUAGE), g_strValue);
        SharedData<bool>(SharedDataType::WEARING_STATE, true);
        EXPECT_EQ(SharedData<bool>::GetData(SharedDataType::WEARING_STATE), true);
    }

    TEST(SharedDataTest, SetDataTest)
    {
        SharedData<uint8_t>(SharedDataType::HEARTBEAT_VALUE, g_intValue, g_intMinValue, g_intMaxValue);
        int newValue = 180;
        SharedData<uint8_t>::SetData(SharedDataType::HEARTBEAT_VALUE, newValue);
        EXPECT_EQ(SharedData<uint8_t>::GetData(SharedDataType::HEARTBEAT_VALUE), newValue);
    }

    TEST(SharedDataTest, AppendNotifyTest)
    {
        SharedData<uint8_t>(SharedDataType::HEARTBEAT_VALUE, g_intValue, g_intMinValue, g_intMaxValue);
        std::thread::id curThreadId = std::this_thread::get_id();
        uint8_t num = 3;
        std::function<void(uint8_t)> func = [&num](uint8_t val) { num += val; };
        SharedData<uint8_t>::AppendNotify(SharedDataType::HEARTBEAT_VALUE, func, curThreadId);
        int newValue = 200;
        int retValue = num + newValue;
        SharedData<uint8_t>::SetData(SharedDataType::HEARTBEAT_VALUE, newValue);
        SharedDataManager::CheckTick();
        EXPECT_EQ(num, retValue);
    }
}