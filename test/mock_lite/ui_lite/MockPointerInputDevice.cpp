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

#include "pointer_input_device.h"
#include "MockGlobalResult.h"

namespace OHOS {
    // all functions only for mock test, no specific implementation
    void PointerInputDevice::DispatchEvent(const DeviceData& data) {}

    void PointerInputDevice::DispatchPressEvent(UIViewGroup* rootView) {}

    bool PointerInputDevice::ProcessReleaseEvent()
    {
        return true;
    }

    void PointerInputDevice::DispatchReleaseEvent(UIViewGroup* rootView) {}

    void PointerInputDevice::DispatchDragStartEvent() {}

    void PointerInputDevice::DispatchDragEvent() {}

    void PointerInputDevice::DispatchDragEndEvent() {}

    void PointerInputDevice::DispatchLongPressEvent(uint32_t elapse) {}

    void PointerInputDevice::DispatchCancelEvent() {}

    void PointerInputDevice::UpdateEventViews(UIView* view) {}

    void PointerInputDevice::OnViewLifeEvent() {}
}