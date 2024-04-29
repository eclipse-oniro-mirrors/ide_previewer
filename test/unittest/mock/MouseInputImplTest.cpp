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
#define private public
#define protected public
#include "MouseInputImpl.h"
#include "MockGlobalResult.h"

namespace {
    TEST(MouseInputImplTest, SetMouseStatusTest)
    {
        int status = 1;
        MouseInputImpl::GetInstance().SetMouseStatus(status);
        EXPECT_EQ(MouseInputImpl::GetInstance().touchAction, status);
    }

    TEST(MouseInputImplTest, SetMousePositionTest)
    {
        double x = 100;
        double y = 200;
        MouseInputImpl::GetInstance().SetMousePosition(x, y);
        EXPECT_EQ(MouseInputImpl::GetInstance().mouseXPosition, x);
        EXPECT_EQ(MouseInputImpl::GetInstance().mouseYPosition, y);
        EXPECT_EQ(MouseInputImpl::GetInstance().GetMouseXPosition(), x);
        EXPECT_EQ(MouseInputImpl::GetInstance().GetMouseYPosition(), y);
    }

    TEST(MouseInputImplTest, SetMouseButtonTest)
    {
        int btn = 1;
        MouseInputImpl::GetInstance().SetMouseButton(btn);
        EXPECT_EQ(MouseInputImpl::GetInstance().pointButton, btn);
    }

    TEST(MouseInputImplTest, SetMouseActionTest)
    {
        int action = 1;
        MouseInputImpl::GetInstance().SetMouseAction(action);
        EXPECT_EQ(MouseInputImpl::GetInstance().pointAction, action);
    }

    TEST(MouseInputImplTest, SetSourceTypeTest)
    {
        int type = 1;
        MouseInputImpl::GetInstance().SetSourceType(type);
        EXPECT_EQ(MouseInputImpl::GetInstance().sourceType, type);
    }

    TEST(MouseInputImplTest, SetSourceToolTest)
    {
        int tool = 1;
        MouseInputImpl::GetInstance().SetSourceTool(tool);
        EXPECT_EQ(MouseInputImpl::GetInstance().sourceTool, tool);
    }

    TEST(MouseInputImplTest, SetPressedBtnsTest)
    {
        std::set<int> pressedBtns;
        MouseInputImpl::GetInstance().SetPressedBtns(pressedBtns);
        EXPECT_EQ(MouseInputImpl::GetInstance().pressedBtnsVec, pressedBtns);
    }

    TEST(MouseInputImplTest, SetAxisValuesTest)
    {
        std::vector<double> axisValues;
        MouseInputImpl::GetInstance().SetAxisValues(axisValues);
        EXPECT_EQ(MouseInputImpl::GetInstance().axisValuesArr, axisValues);
    }

    TEST(MouseInputImplTest, DispatchOsTouchEventTest)
    {
        g_dispatchPointerEvent = false;
        MouseInputImpl::GetInstance().DispatchOsTouchEvent();
        EXPECT_TRUE(g_dispatchPointerEvent);
    }

    TEST(MouseInputImplTest, DispatchOsBackEventTest)
    {
        g_dispatchBackPressedEvent = false;
        MouseInputImpl::GetInstance().DispatchOsBackEvent();
        EXPECT_TRUE(g_dispatchBackPressedEvent);
    }

    TEST(MouseInputImplTest, ConvertToOsTypeTest)
    {
        int typeNum = -1;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::UNKNOWN);

        typeNum = 10;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::UNKNOWN);

        typeNum = 0;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::DOWN);

        typeNum = 1;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::UP);

        typeNum = 2;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::MOVE);

        typeNum = 3;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::CANCEL);

        typeNum = 4;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::PULL_DOWN);

        typeNum = 5;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::PULL_UP);

        typeNum = 6;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::PULL_MOVE);

        typeNum = 7;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::PULL_IN_WINDOW);

        typeNum = 8;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::PULL_OUT_WINDOW);

        typeNum = 9;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsType(typeNum), OHOS::MMI::TouchType::UNKNOWN);
    }

    TEST(MouseInputImplTest, ConvertToOsToolTest)
    {
        int typeNum = -1;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsTool(typeNum), OHOS::MMI::SourceTool::UNKNOWN);

        typeNum = 10;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsTool(typeNum), OHOS::MMI::SourceTool::UNKNOWN);

        typeNum = 0;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsTool(typeNum), OHOS::MMI::SourceTool::UNKNOWN);

        typeNum = 1;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsTool(typeNum), OHOS::MMI::SourceTool::FINGER);

        typeNum = 2;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsTool(typeNum), OHOS::MMI::SourceTool::PEN);

        typeNum = 3;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsTool(typeNum), OHOS::MMI::SourceTool::RUBBER);

        typeNum = 4;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsTool(typeNum), OHOS::MMI::SourceTool::BRUSH);

        typeNum = 5;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsTool(typeNum), OHOS::MMI::SourceTool::PENCIL);

        typeNum = 6;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsTool(typeNum), OHOS::MMI::SourceTool::AIRBRUSH);

        typeNum = 7;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsTool(typeNum), OHOS::MMI::SourceTool::MOUSE);

        typeNum = 8;
        EXPECT_EQ(MouseInputImpl::GetInstance().ConvertToOsTool(typeNum), OHOS::MMI::SourceTool::LENS);
    }
}