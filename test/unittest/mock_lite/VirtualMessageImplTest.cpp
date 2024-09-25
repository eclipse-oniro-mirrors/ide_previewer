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
#include "VirtualMessageImpl.h"

namespace {
    bool g_executeSuccess = false;

    int32_t SuccessCallback(const void *data)
    {
        g_executeSuccess = true;
        return 0;
    }

    int32_t FailCallback(const void *data, uint16_t dataLength, uint16_t errorCode)
    {
        return 0;
    }

    TEST(VirtualMessageImplTest, RegistBundleTest)
    {
        const char* key = "test";
        VirtualMessageImpl::GetInstance().RegistBundle(key, SuccessCallback, FailCallback);
        EXPECT_TRUE(VirtualMessageImpl::GetInstance().callBacks.find(key) !=
            VirtualMessageImpl::GetInstance().callBacks.end());
    }

    TEST(VirtualMessageImplTest, UnregistBundleTest)
    {
        const char* key = "test";
        VirtualMessageImpl::GetInstance().UnregistBundle(key);
        EXPECT_TRUE(VirtualMessageImpl::GetInstance().callBacks.find(key) ==
            VirtualMessageImpl::GetInstance().callBacks.end());
    }

    TEST(VirtualMessageImplTest, StringToCharVectorTest)
    {
        std::string test = "123";
        const char* testArr = test.c_str();
        std::vector<char> vec = VirtualMessageImpl::GetInstance().StringToCharVector("123");
        for (int i = 0; i < test.size(); i++) {
            EXPECT_EQ(vec[i], testArr[i]);
        }
    }

    TEST(VirtualMessageImplTest, SendVirtualMessageTest)
    {
        MessageInfo info;
        info.deviceID = "1234";
        info.bundleName = "index";
        info.abilityName = "entry";
        info.message = "test";
        const char* key = "test";
        VirtualMessageImpl::GetInstance().RegistBundle(key, SuccessCallback, FailCallback);
        VirtualMessageImpl::GetInstance().SendVirtualMessage(info);
        EXPECT_TRUE(g_executeSuccess);
        VirtualMessageImpl::GetInstance().UnregistBundle(key);
    }
}