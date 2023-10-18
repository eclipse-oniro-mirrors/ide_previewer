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

MouseInputImpl::MouseInputImpl()
{
}

MouseInputImpl& MouseInputImpl::GetInstance()
{
    static MouseInputImpl instance;
    return instance;
}

bool MouseInputImpl::Read(OHOS::DeviceData& data)
{
    data.point.x = static_cast<short>(mouseXPosition);
    data.point.y = static_cast<short>(mouseYPosition);
    data.state = touchAction;
    return false;
}

void MouseInputImpl::SetMouseStatus(int status)
{
    int moveType = 2;
    if (status == moveType) {
        return;
    }
    int xorFlag = 1;
    touchAction = status ^ xorFlag; // lite and rich touch type is opposite
}

