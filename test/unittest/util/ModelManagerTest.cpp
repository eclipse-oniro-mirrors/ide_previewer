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
#include "ModelManager.h"

namespace {
    // 测试默认构造函数是否被删除
    TEST(ModelManagerTest, DefaultConstructorDeletedTest)
    {
        EXPECT_TRUE(std::is_default_constructible<ModelManager>::value == false);
    }

    // 测试赋值运算符是否被删除
    TEST(ModelManagerTest, AssignmentOperatorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_assignable<ModelManager>::value == false);
    }

    TEST(ModelManagerTest, SetCurrentDeviceTest)
    {
        std::string device = "liteWearable";
        ModelManager::SetCurrentDevice(device);
        EXPECT_EQ(ModelManager::GetCurrentModel(), device);
        const ModelConfig& config = ModelManager::GetConfig();
        EXPECT_EQ(config.deviceType, device);
    }

    TEST(ModelManagerTest, GetAllModelNameTest)
    {
        std::string devices = "*** *** ";
        std::string names = ModelManager::GetAllModelName();
        EXPECT_EQ(names, devices);
    }

    TEST(ModelManagerTest, GetConfigByDeviceTest)
    {
        std::string device = "smartVision";
        const ModelConfig& config = ModelManager::GetConfig(device);
        EXPECT_EQ(config.deviceType, device);
    }
}