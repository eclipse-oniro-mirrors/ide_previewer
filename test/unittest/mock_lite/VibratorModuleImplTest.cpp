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
#include "vibrator_impl.h"

namespace {
    TEST(VibratorModuleImplTest, VibrateTest)
    {
        const char* vibMode1 = "long";
        int32_t ret1 = Vibrate(vibMode1);
        EXPECT_EQ(ret1, 0);

        const char* vibMode2 = "short";
        int32_t ret2 = Vibrate(vibMode2);
        EXPECT_EQ(ret2, 0);

        const char* vibMode3 = "aaa";
        int32_t ret3 = Vibrate(vibMode3);
        EXPECT_EQ(ret3, 0);
    }
}