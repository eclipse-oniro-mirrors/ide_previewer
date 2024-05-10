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
    std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const {}
    Rect GetRect() const {}
    const std::shared_ptr<AbilityRuntime::Context> GetContext() const {}
    WindowMode GetMode() const {}
    float GetAlpha() const {}
    Rect GetRequestRect() const {}
    WindowType GetType() const {}
    const std::string& GetWindowName() const {}
    WMError SetFocusable(bool isFocusable) {}
    bool GetFocusable() const {}
    WMError SetTouchable(bool isTouchable) {}
    uint32_t GetWindowId() const {}
    uint32_t GetWindowFlags() const {}
    WindowState GetWindowState() const {}
    SystemBarProperty GetSystemBarPropertyByType(WindowType type) const {}
    WMError SetWindowMode(WindowMode mode) {}
    WMError SetWindowType(WindowType type) {}
    bool GetTouchable() const {}
    bool IsLayoutFullScreen() const {}
    bool IsFullScreen() const {}
    WMError SetAlpha(float alpha) {}
    WMError AddWindowFlag(WindowFlag flag) {}
    WMError SetWindowFlags(uint32_t flags) {}
    WMError SetTransform(const Transform& trans) {}
    const Transform& GetTransform() const {}
    WMError RemoveWindowFlag(WindowFlag flag) {}
    WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea) {}
    WMError SetLayoutFullScreen(bool status) {}
    WMError Destroy() {}
    WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property) {}
    WMError SetSpecificBarProperty(WindowType type, const SystemBarProperty& property) {}
    WMError SetFullScreen(bool status) {}
    WMError Show(uint32_t reason = 0, bool withAnimation = false) {}
    WMError Hide(uint32_t reason = 0, bool withAnimation = false, bool isFromInnerkits = true) {}
    WMError SetWindowGravity(WindowGravity gravity, uint32_t percent) {}
    WMError SetKeepScreenOn(bool keepScreenOn) {}
    bool IsKeepScreenOn() const {}
    WMError MoveTo(int32_t x, int32_t y) {}
    WMError Resize(uint32_t width, uint32_t height) {}
    WMError SetTurnScreenOn(bool turnScreenOn) {}
    bool IsTurnScreenOn() const {}
    bool IsTransparent() const {}
    WMError SetBrightness(float brightness) {}
    WMError SetBackgroundColor(const std::string& color) {}
    WMError SetTransparent(bool isTransparent) {}
    float GetBrightness() const {}
    bool IsPrivacyMode() const {}
    void SetSystemPrivacyMode(bool isSystemPrivacyMode) {}
    WMError SetCallingWindow(uint32_t windowId) {}
    WMError SetPrivacyMode(bool isPrivacyMode) {}
    WMError BindDialogTarget(sptr<IRemoteObject> targetToken) {}
    WmErrorCode RaiseToAppTop() {}
    WMError SetShadowRadius(float radius) {}
    WMError SetShadowColor(std::string color) {}
    WMError SetSnapshotSkip(bool isSkip) {}
    WMError SetCornerRadius(float cornerRadius) {}
    WMError SetShadowOffsetX(float offsetX) {}
    WMError SetBackdropBlurStyle(WindowBlurStyle blurStyle) {}
    WMError RequestFocus() const {}
    WMError SetShadowOffsetY(float offsetY) {}
    WMError SetBlur(float radius) {}
    WMError SetBackdropBlur(float radius) {}
    bool IsFocused() const {}
    void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& inputEvent) {}
    void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) {}
    int64_t GetVSyncPeriod() {}
    WMError UpdateSurfaceNodeAfterCustomAnimation(bool isAdd) {}
    void SetInputEventConsumer(const std::shared_ptr<IInputEventConsumer>& inputEventConsumer) {}
    void ConsumeKeyEvent(const std::shared_ptr<MMI::KeyEvent>& inputEvent) {}
    void FlushFrameRate(uint32_t rate, bool isAnimatorStopped) {}
    WMError UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) {}
    WMError RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) {}
    void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) {}
    WMError RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) {}
    WMError UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) {}
    WMError RegisterDragListener(const sptr<IWindowDragListener>& listener) {}
    WMError UnregisterDragListener(const sptr<IWindowDragListener>& listener) {}
    WMError RegisterAvoidAreaChangeListener(sptr<IAvoidAreaChangedListener>& listener) {}
    WMError UnregisterAvoidAreaChangeListener(sptr<IAvoidAreaChangedListener>& listener) {}
    WMError RegisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) {}
    WMError UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) {}
    WMError RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) {}
    WMError UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) {}
    WMError UnregisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) {}
    void RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func) {}
    WMError RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) {}
    WMError RegisterAnimationTransitionController(const sptr<IAnimationTransitionController>& listener) {}
    WMError RegisterScreenshotListener(const sptr<IScreenshotListener>& listener) {}
    void RegisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) {}
    void UnregisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) {}
    void NotifyTouchDialogTarget(int32_t posX = 0, int32_t posY = 0) {}
    WMError UnregisterScreenshotListener(const sptr<IScreenshotListener>& listener) {}
    WMError RegisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) {}
    WMError UnregisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) {}
    void SetAceAbilityHandler(const sptr<IAceAbilityHandler>& handler) {}
    WMError SetUIContentByAbc(const std::string& abcPath, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability = nullptr)
    {
        return WMError::WM_OK;
    }
    WMError NapiSetUIContent(const std::string& contentInfo, napi_env env,
        napi_value storage, bool isDistributed = false, sptr<IRemoteObject> token = nullptr,
        AppExecFwk::Ability* ability = nullptr) {}
    WMError SetUIContentByName(const std::string& contentInfo, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability = nullptr)
    {
        return WMError::WM_OK;
    }
    Ace::UIContent* GetUIContent() const {}
    void OnNewWant(const AAFwk::Want& want) {}
    void SetRequestedOrientation(Orientation) {}
    std::string GetContentInfo() {}
    Orientation GetRequestedOrientation() {}
    WMError SetTouchHotAreas(const std::vector<Rect>& rects) {}
    void GetRequestedTouchHotAreas(std::vector<Rect>& rects) const {}
    bool IsMainHandlerAvailable() const {}
    void SetRequestModeSupportInfo(uint32_t modeSupportInfo) {}
    uint32_t GetRequestModeSupportInfo() const {}
    WMError SetAPPWindowLabel(const std::string& label) {}
    WMError SetAPPWindowIcon(const std::shared_ptr<Media::PixelMap>& icon) {}
    WMError Recover() {}
    void StartMove() {}
    WMError Close() {}
    WMError DisableAppWindowDecor() {}
    WMError Minimize() {}
    WMError Maximize() {}
    void SetNeedRemoveWindowInputChannel(bool needRemoveWindowInputChannel) {}
    ColorSpace GetColorSpace() {}
    void DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info) {}
    std::shared_ptr<Media::PixelMap> Snapshot() {}
    bool IsSupportWideGamut() {}
    void SetColorSpace(ColorSpace colorSpace) {}
    WMError NotifyMemoryLevel(int32_t level) {}
    WMError ResetAspectRatio() {}
    KeyboardAnimationConfig GetKeyboardAnimationConfig() {}
    void SetNeedDefaultAnimation(bool needDefaultAnimation) {}
    bool IsAllowHaveSystemSubWindow() {}
    WMError SetAspectRatio(float ratio) {}

    void SetViewportConfig(const Ace::ViewportConfig& config) {}
    void UpdateViewportConfig() {}
    void SetSize(int32_t width, int32_t height) {}
    void SetDensity(float density) {}
    void SetOrientation(Orientation orientation) {}

    void CreateSurfaceNode(const std::string name, const SendRenderDataCallback& callback)
    {
        g_createSurfaceNode = true;
    }
    WmErrorCode RaiseAboveTarget(int32_t subWindowId) {}
    WMError HideNonSystemFloatingWindows(bool shouldHide) {}
    bool IsFloatingWindowAppType() const { return false; }
    void SetContentInfoCallback(const ContentInfoCallback& callback)
    {
        g_setContentInfoCallback = true;
    }
    WMError SetResizeByDragEnabled(bool dragEnabled) {}
    WMError SetRaiseByClickEnabled(bool raiseEnabled) {}
    WmErrorCode KeepKeyboardOnFocus(bool keepKeyboardFlag) {}
    WMError RegisterWindowVisibilityChangeListener(const WindowVisibilityListenerSptr& listener) {}
    WMError UnregisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    WMError UnregisterWindowVisibilityChangeListener(const WindowVisibilityListenerSptr& listener) {}
    WMError SetWindowLimits(WindowLimits& windowLimits) { return WMError::WM_OK; };
    WMError GetWindowLimits(WindowLimits& windowLimits) { return WMError::WM_OK; };
    WMError RegisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    WMError SetDecorHeight(int32_t decorHeight) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    WMError GetDecorHeight(int32_t& height) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    WMError SetSingleFrameComposerEnabled(bool enable) {}
    WMError SetLandscapeMultiWindow(bool isLandscapeMultiWindow) {}
    WMError SetDecorVisible(bool isVisible) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    WMError RegisterWindowTitleButtonRectChangeListener(
        const sptr<IWindowTitleButtonRectChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    WMError GetTitleButtonArea(TitleButtonRect& titleButtonRect)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    WMError SetSubWindowModal(bool isModal)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    WMError UnregisterWindowTitleButtonRectChangeListener(
        const sptr<IWindowTitleButtonRectChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    WMError Maximize(MaximizeLayoutOption option) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    WMError Recover(uint32_t reason = 0) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; };
};

sptr<Window> Window::Create(const std::string& windowName, sptr<WindowOption>& option,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context, WMError& errCode)
{
    sptr<Window> windowImpl = new(std::nothrow) MockWindow();
    return windowImpl;
}
}
}