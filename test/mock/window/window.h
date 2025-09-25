/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_WINDOW_H
#define OHOS_ROSEN_WINDOW_H

#include <refbase.h>
#include <iremote_object.h>
#include "window_option.h"

namespace OHOS::MMI {
class PointerEvent;
class KeyEvent;
class AxisEvent;
}

namespace OHOS::Ace {
class UIContent;
class ViewportConfig;
}

namespace OHOS::AbilityRuntime {
class AbilityContext;
class Context;
}

namespace OHOS {
namespace Rosen {
class RSUIContext;
using ContentInfoCallback = std::function<void(std::string contentInfo)>;
using SendRenderDataCallback = bool (*)(const void*, const size_t, const int32_t, const int32_t, const uint64_t);

class IIgnoreViewSafeAreaListener : virtual public RefBase {
public:
    void SetIgnoreViewSafeArea(bool ignoreViewSafeArea) {}
};

class IAvoidAreaChangedListener : virtual public RefBase {
public:
    void OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type) {}
};

class IWindowSystemBarEnableListener : virtual public RefBase {
public:
    WMError OnSetSpecificBarProperty(WindowType type, const SystemBarProperty& property)
    {
        return WMError::WM_OK;
    }
};

static WMError DefaultCreateErrCode = WMError::WM_OK;
class Window : public RefBase {
public:
    static sptr<Window> Create(const std::string& windowName,
    sptr<WindowOption>& option, const std::shared_ptr<AbilityRuntime::Context>& context = nullptr,
    WMError& errCode = DefaultCreateErrCode, const std::shared_ptr<RSUIContext>& rsUIContext = nullptr);

    virtual WMError RegisterSystemBarEnableListener(const sptr<IWindowSystemBarEnableListener>& listener) = 0;
    virtual WMError UnRegisterSystemBarEnableListener(const sptr<IWindowSystemBarEnableListener>& listener) = 0;
    virtual void SetContentInfoCallback(const ContentInfoCallback& callback) = 0;
    virtual void CreateSurfaceNode(const std::string name, const SendRenderDataCallback& callback) = 0;
    virtual void SetViewportConfig(const Ace::ViewportConfig& config) = 0;
    virtual void ConsumeKeyEvent(const std::shared_ptr<MMI::KeyEvent>& inputEvent) = 0;
    virtual void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& inputEvent) = 0;
    virtual void UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type) = 0;
    virtual SystemBarProperty GetSystemBarPropertyByType(WindowType type) const = 0;
};
}
}
#endif // OHOS_ROSEN_WINDOW_H
