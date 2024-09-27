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
#define protected public
#include "MouseInputImpl.h"

namespace {
    double g_xPosition = 10;
    double g_yPosition = 20;
    int g_action = 0;
    int g_xorFlag = 1;

    class MouseInputImplTest : public ::testing::Test {
    public:
        MouseInputImplTest() {}
        ~MouseInputImplTest() {}
    protected:
        static void SetUpTestCase()
        {
            MouseInputImpl::GetInstance().SetMousePosition(g_xPosition, g_yPosition);
            MouseInputImpl::GetInstance().SetMouseStatus(g_action);
        }
    };

    TEST_F(MouseInputImplTest, ReadTest)
    {
        OHOS::DeviceData data;
        MouseInputImpl::GetInstance().Read(data);
        EXPECT_EQ(data.point.x, g_xPosition);
        EXPECT_EQ(data.point.y, g_yPosition);
        int ret = g_action ^ g_xorFlag;
        EXPECT_EQ(data.state, ret);
    }

    TEST_F(MouseInputImplTest, SetMouseStatusTest)
    {
        int status = 2;
        MouseInputImpl::GetInstance().SetMouseStatus(status);
        int ret = g_action ^ g_xorFlag;
        EXPECT_EQ(MouseInputImpl::GetInstance().touchAction, ret);

        status = 1;
        MouseInputImpl::GetInstance().SetMouseStatus(status);
        EXPECT_EQ(MouseInputImpl::GetInstance().touchAction, 0);
    }
}