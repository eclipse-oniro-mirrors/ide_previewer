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
#include "LocalDate.h"
using namespace std;

namespace {
    TEST(LocalDateTest, IsBigEndianTest)
    {
        time_t inputTime = time(nullptr); // 获取当前时间
        struct tm utcTime;
        LocalDate::GmTimeSafe(utcTime, inputTime);
        EXPECT_GT(utcTime.tm_year + 1900, 1970);
    }
}