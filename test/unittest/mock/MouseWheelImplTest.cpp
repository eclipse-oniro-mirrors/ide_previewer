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
#include "MouseWheelImpl.h"

namespace {
    TEST(MouseWheelImplTest, SetMousePositionTest)
    {
        double x = 100;
        double y = 200;
        MouseWheelImpl::GetInstance().SetMousePosition(x, y);
        EXPECT_EQ(MouseWheelImpl::GetInstance().GetMouseXPosition(), x);
        EXPECT_EQ(MouseWheelImpl::GetInstance().GetMouseYPosition(), y);
    }

    TEST(MouseWheelImplTest, SetRotateTest)
    {
        double rotate = 100;
        MouseWheelImpl::GetInstance().SetRotate(rotate);
        EXPECT_EQ(MouseWheelImpl::GetInstance().GetRotate(), rotate);
    }
}