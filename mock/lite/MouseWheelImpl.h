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

#ifndef MOUSEWHEELIMPL_H
#define MOUSEWHEELIMPL_H

#include "rotate_input_device.h"

#include "MouseWheel.h"

class MouseWheelImpl : public MouseWheel, public OHOS::RotateInputDevice {
public:
    static MouseWheelImpl& GetInstance();
    bool Read(OHOS::DeviceData& data) override;

private:
    MouseWheelImpl();
    virtual ~MouseWheelImpl() {}
};

#endif // MOUSEWHEELIMPL_H
