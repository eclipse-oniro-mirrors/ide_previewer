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

#include <string>
#include <vector>
#include "gtest/gtest.h"
#define protected public
#include "LanguageManagerImpl.h"

namespace {
    // 测试拷贝构造函数是否被删除
    TEST(LanguageManagerImplTest, CopyConstructorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_constructible<LanguageManagerImpl>::value == false);
    }

    // 测试赋值运算符是否被删除
    TEST(LanguageManagerImplTest, AssignmentOperatorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_assignable<LanguageManagerImpl>::value == false);
    }

    TEST(LanguageManagerImplTest, GetSupportedLanguagesTest)
    {
        std::list<std::string> list = LanguageManagerImpl::GetInstance().GetSupportedLanguages();
        EXPECT_EQ(list.size(), 0);
    }
}