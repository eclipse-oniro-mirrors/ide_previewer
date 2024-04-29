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
#include "KeyInputImpl.h"
#include "MockGlobalResult.h"

namespace {
    TEST(KeyInputImplTest, DispatchOsInputMethodEventTest)
    {
        int code = 10;
        KeyInputImpl::GetInstance().SetCodePoint(code);
        EXPECT_EQ(KeyInputImpl::GetInstance().codePoint, code);
        g_dispatchInputMethodEvent = false;
        KeyInputImpl::GetInstance().DispatchOsInputMethodEvent();
        EXPECT_TRUE(g_dispatchInputMethodEvent);
    }

    TEST(KeyInputImplTest, DispatchOsKeyEventTest)
    {
        int32_t keyCode = 2047;
        int32_t keyAction = 1;
        std::vector<int32_t> pressedCodesVec;
        for (unsigned int i = 0; i < 3; i++) {
            pressedCodesVec.push_back(keyCode + i);
        }
        std::string keyString = "ctrl";
        KeyInputImpl::GetInstance().SetKeyEvent(keyCode, keyAction, pressedCodesVec, keyString);
        EXPECT_EQ(KeyInputImpl::GetInstance().keyCode, keyCode);
        EXPECT_EQ(KeyInputImpl::GetInstance().keyAction, keyAction);
        EXPECT_EQ(KeyInputImpl::GetInstance().keyString, keyString);

        g_dispatchKeyEvent = false;
        KeyInputImpl::GetInstance().DispatchOsKeyEvent();
        EXPECT_TRUE(g_dispatchKeyEvent);
    }
}