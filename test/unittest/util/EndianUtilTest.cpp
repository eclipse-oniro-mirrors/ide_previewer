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
#include "EndianUtil.h"
#include <string>
#include "gtest/gtest.h"

namespace {
    TEST(EndianUtilTest, IsBigEndianTest)
    {
        // 测试 IsBigEndian() 方法是否返回正确结果
        bool ret = EndianUtil::IsBigEndian();
        EXPECT_FALSE(ret); // 当前环境为小端序
    }

    TEST(EndianUtilTest, ToNetworkEndianTest)
    {
        // 测试 ToNetworkEndian() 方法是否正确转换字节序
        uint32_t value = 0x12345678;
        uint32_t networkValue = EndianUtil::ToNetworkEndian(value);
        if (EndianUtil::IsBigEndian()) {
            EXPECT_EQ(networkValue, value); // 当前为大端环境，不转换
        } else {
            EXPECT_EQ(networkValue, 0x78563412); // 在小端序环境下应该转换为 0x78563412
        }
    }
}