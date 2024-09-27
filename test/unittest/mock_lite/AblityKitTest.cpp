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
#include "ability_kit.h"
#define private public
#include "VirtualMessageImpl.h"

namespace {
    int32_t MessageSuccessCallback(const void *data)
    {
        return 0;
    }

    int32_t MessageFailCallback(const void *data, uint16_t dataLength, uint16_t errorCode)
    {
        return 0;
    }

    TEST(AblityKitTest, RegisterReceiverTest)
    {
        int32_t ret = OHOS::AbilityKit::RegisterReceiver("testBundleName",
            MessageSuccessCallback, MessageFailCallback, 0, 0, 0);
        EXPECT_FALSE(VirtualMessageImpl::GetInstance().callBacks.empty());
        EXPECT_EQ(ret, 0);
    }

    TEST(AblityKitTest, UnregisterReceiverTest)
    {
        int32_t ret = OHOS::AbilityKit::UnregisterReceiver("testBundleName");
        EXPECT_TRUE(VirtualMessageImpl::GetInstance().callBacks.empty());
        EXPECT_EQ(ret, 0);
    }

    TEST(AblityKitTest, SendMsgToPeerAppTest)
    {
        int32_t ret = OHOS::AbilityKit::SendMsgToPeerApp(false, "", "", 0, 0, 0, 0, 0);
        EXPECT_EQ(ret, 0);
    }

    TEST(AblityKitTest, DetectPhoneAppTest)
    {
        int32_t ret = OHOS::AbilityKit::DetectPhoneApp("", "", 0, 0, 0);
        EXPECT_EQ(ret, 0);
    }

    TEST(AblityKitTest, DetectResourceReleaseTest)
    {
        int32_t ret = OHOS::AbilityKit::DetectResourceRelease();
        EXPECT_EQ(ret, 0);
    }

    TEST(AblityKitTest, SendMsgResourceReleaseTest)
    {
        int32_t ret = OHOS::AbilityKit::SendMsgResourceRelease();
        EXPECT_EQ(ret, 0);
    }
}