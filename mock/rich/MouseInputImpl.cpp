/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "MouseInputImpl.h"

#include <thread>
#include <vector>
#include <chrono>
#include <sstream>

#include "PreviewerEngineLog.h"

using namespace std;

using namespace OHOS::MMI;

MouseInputImpl::MouseInputImpl() noexcept
{
}

TouchType MouseInputImpl::ConvertToOsType(int status) const
{
    if (status < static_cast<int>(TouchType::DOWN) || status > static_cast<int>(TouchType::UNKNOWN)) {
        return TouchType::UNKNOWN;
    }
    return static_cast<TouchType>(status);
}

SourceTool MouseInputImpl::ConvertToOsTool(int tools) const
{
    if (tools < static_cast<int>(SourceTool::UNKNOWN) || tools > static_cast<int>(SourceTool::LENS)) {
        return SourceTool::UNKNOWN;
    }
    return static_cast<SourceTool>(tools);
}

void MouseInputImpl::DispatchOsTouchEvent() const
{
    auto pointerEvent = std::make_shared<PointerEvent>();
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    std::chrono::duration<int64_t, std::nano> duration(ts.tv_sec * SEC_TO_NANOSEC + ts.tv_nsec);
    pointerEvent->time = std::chrono::high_resolution_clock::time_point(duration);
    pointerEvent->id = 1;
    pointerEvent->x = mouseXPosition;
    pointerEvent->y = mouseYPosition;
    pointerEvent->type = ConvertToOsType(touchAction);
    pointerEvent->buttonId_ = pointButton;
    pointerEvent->pointerAction_ = pointAction;
    pointerEvent->sourceType = sourceType;
    pointerEvent->sourceTool = ConvertToOsTool(sourceTool);
    pointerEvent->pressedButtons_ = pressedBtnsVec;
    std::copy(axisValuesArr.begin(), axisValuesArr.end(), pointerEvent->axisValues_.begin());
    pointerEvent->size = sizeof (PointerEvent);
    std::stringstream ss;
    ss << "[";
    for (double val : axisValuesArr) {
        ss << " " << val << " ";
    }
    ss << "]" << std::endl;
    ILOG("MouseInputImpl::DispatchEvent x: %f y:%f type:%d buttonId_:%d pointerAction_:%d sourceType:%d \
        sourceTool:%d pressedButtonsSize:%d axisValuesArr:%s", pointerEvent->x, pointerEvent->y,
        pointerEvent->type, pointerEvent->buttonId_, pointerEvent->pointerAction_, pointerEvent->sourceType,
        pointerEvent->sourceTool, pointerEvent->pressedButtons_.size(), ss.str().c_str());
    ILOG("current thread: %d", this_thread::get_id());
    JsAppImpl::GetInstance().DispatchPointerEvent(pointerEvent);
}

void MouseInputImpl::DispatchOsBackEvent() const
{
    ILOG("DispatchBackPressedEvent run.");
    ILOG("current thread: %d", this_thread::get_id());
    JsAppImpl::GetInstance().DispatchBackPressedEvent();
}

MouseInputImpl& MouseInputImpl::GetInstance()
{
    static MouseInputImpl instance;
    return instance;
}

void MouseInputImpl::SetMouseStatus(int status)
{
    touchAction = status;
}

void MouseInputImpl::SetMousePosition(double xPosition, double yPosition)
{
    mouseXPosition = xPosition;
    mouseYPosition = yPosition;
}
