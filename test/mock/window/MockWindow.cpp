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

#include "window.h"
#include "MockGlobalResult.h"

namespace OHOS {
namespace Rosen {
class MockWindow : public Window {
public:
    WMError RegisterSystemBarEnableListener(const sptr<IWindowSystemBarEnableListener>& listener) override
    {
        return WMError::WM_OK;
    }
    WMError UnRegisterSystemBarEnableListener(const sptr<IWindowSystemBarEnableListener>& listener) override
    {
        return WMError::WM_OK;
    }
    void SetContentInfoCallback(const ContentInfoCallback& callback) override
    {
        contentInfoCallback = callback;
        g_setContentInfoCallback = true;
    }
    void CreateSurfaceNode(const std::string name, const SendRenderDataCallback& callback) override
    {
        g_createSurfaceNode = true;
    }
    void SetViewportConfig(const Ace::ViewportConfig& config) override {}
    void ConsumeKeyEvent(const std::shared_ptr<MMI::KeyEvent>& inputEvent) override {}
    void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& inputEvent) override {}
    void UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type) override
    {
        g_updateAvoidArea = true;
    }
    SystemBarProperty GetSystemBarPropertyByType(WindowType type) const override
    {
        g_getSystemBarPropertyByType = true;
        return property;
    }
private:
    SystemBarProperty property;
    ContentInfoCallback contentInfoCallback;
};

sptr<Window> Window::Create(const std::string& windowName, sptr<WindowOption>& option,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context, WMError& errCode)
{
    sptr<MockWindow> windowImpl = new(std::nothrow) MockWindow();
    return windowImpl;
}
}
}