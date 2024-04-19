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

#include "MouseInputImpl.h"
#include "MockGlobalResult.h"

MouseInputImpl::MouseInputImpl() noexcept {}

MouseInputImpl& MouseInputImpl::GetInstance()
{
    static MouseInputImpl instance;
    return instance;
}

void MouseInputImpl::SetMousePosition(double xPosition, double yPosition)
{
    mouseXPosition = xPosition;
    mouseYPosition = yPosition;
}

void MouseInputImpl::SetMouseStatus(int status)
{
    touchAction = status;
}

void MouseInputImpl::DispatchOsTouchEvent() const
{
    g_dispatchOsTouchEvent = true;
}

void MouseInputImpl::DispatchOsBackEvent() const
{
    g_dispatchOsBackEvent = true;
}