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
#include "TimeTool.h"

namespace {
    TEST(TimeToolTest, GetFormatTimeTest)
    {
        // eg: [2024-04-11T14:50:00.556]
        std::string formattedTime = TimeTool::GetFormatTime().c_str();
        ASSERT_FALSE(formattedTime.empty());
        int len = formattedTime.size() - 1;
        ASSERT_EQ(formattedTime[0], '[');
        ASSERT_EQ(formattedTime[len], ']');
        ASSERT_EQ(formattedTime[5], '-');
        ASSERT_EQ(formattedTime[8], '-');
        ASSERT_EQ(formattedTime[11], 'T');
        ASSERT_EQ(formattedTime[14], ':');
        ASSERT_EQ(formattedTime[17], ':');
        ASSERT_EQ(formattedTime[20], '.');
    }

    TEST(TimeToolTest, GetTraceFormatTimeTest)
    {
        // eg: 2024-04-11T14:58:00.317
        std::string formattedTime = TimeTool::GetTraceFormatTime().c_str();
        ASSERT_FALSE(formattedTime.empty());
        int len = formattedTime.size() - 1;
        ASSERT_EQ(formattedTime[4], '-');
        ASSERT_EQ(formattedTime[7], '-');
        ASSERT_EQ(formattedTime[10], 'T');
        ASSERT_EQ(formattedTime[13], ':');
        ASSERT_EQ(formattedTime[16], ':');
        ASSERT_EQ(formattedTime[19], '.');
    }
}