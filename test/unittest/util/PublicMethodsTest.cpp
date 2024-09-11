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
#include "PublicMethods.h"

namespace {
    // 测试拷贝构造函数是否被删除
    TEST(CppTimerTest, CopyConstructorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_constructible<PublicMethods>::value == false);
    }

    // 测试赋值运算符是否被删除
    TEST(CppTimerTest, AssignmentOperatorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_assignable<PublicMethods>::value == false);
    }

    // 比较两个 int8_t 数组是否相等
    bool CompareInt8Arrays(const int8_t* arr1, const int8_t* arr2, size_t size)
    {
        for (size_t i = 0; i < size; ++i) {
            if (arr1[i] != arr2[i]) {
                return false;
            }
        }
        return true;
    }

    TEST(PublicMethodsTest, UlltoaTest)
    {
        const int testValue = 123456789;
        const int8_t expectedOutput[] = "75bcd15";

        int8_t outputBuffer[PublicMethods::MAX_ITOA_BIT] = {0};
        uint32_t resultLength = PublicMethods::Ulltoa(testValue, outputBuffer);
        // 验证返回值是否正确
        ASSERT_EQ(std::size(expectedOutput) - 1, resultLength); // 减去 1，以排除空字符(‘\0’)的影响
        // 验证结果字符串是否与预期相符
        ASSERT_TRUE(CompareInt8Arrays(expectedOutput, outputBuffer, resultLength));
    }
}