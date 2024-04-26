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

#include "JsAppImpl.h"
#include "MockGlobalResult.h"

using namespace std;

JsAppImpl::JsAppImpl() noexcept : ability(nullptr), isStop(false) {}

JsAppImpl::~JsAppImpl() {}

JsAppImpl& JsAppImpl::GetInstance()
{
    static JsAppImpl instance;
    return instance;
}

void JsAppImpl::ResolutionChanged(ResolutionParam& param, int32_t screenDensity, string reason)
{
    this->width = param.orignalWidth;
    this->height = param.orignalHeight;
}

std::string JsAppImpl::GetOrientation() const
{
    g_getOrientation = true;
    return orientation;
}

void JsAppImpl::OrientationChanged(std::string commandOrientation)
{
    orientation = commandOrientation;
}

std::string JsAppImpl::GetColorMode() const
{
    g_getColorMode = true;
    return colorMode;
}

void JsAppImpl::SetArgsColorMode(const string& value)
{
    colorMode = value;
}

void JsAppImpl::ColorModeChanged(const std::string commandColorMode)
{
    colorMode = commandColorMode;
}

bool JsAppImpl::MemoryRefresh(const std::string memoryRefreshArgs) const
{
    g_memoryRefresh = true;
    return true;
}

void JsAppImpl::LoadDocument(const std::string filePath, const std::string componentName,
    const Json2::Value& previewContext)
{
    g_loadDocument = true;
}

void JsAppImpl::ReloadRuntimePage(const std::string currentPage)
{
    g_reloadRuntimePage = true;
}

void JsAppImpl::Restart()
{
    g_restart = true;
}

std::string JsAppImpl::GetJSONTree()
{
    g_getJSONTree = true;
    return "";
}

std::string JsAppImpl::GetDefaultJSONTree()
{
    g_getDefaultJSONTree = true;
    return "";
}

void JsAppImpl::FoldStatusChanged(const std::string commandFoldStatus, int32_t width, int32_t height)
{
    this->width = width;
    this->height = height;
}

void JsAppImpl::Start()
{
    isFinished = true;
}

void JsAppImpl::Interrupt()
{
    isStop = true;
}
void JsAppImpl::SetArgsAceVersion(const std::string& value)
{
    aceVersion = value;
}

void JsAppImpl::SetScreenDensity(const std::string value)
{
    screenDensity = value;
}

void JsAppImpl::SetConfigChanges(const std::string value)
{
    configChanges = value;
}

void JsAppImpl::DispatchBackPressedEvent() const
{
    g_dispatchBackPressedEvent = true;
}

void JsAppImpl::DispatchPointerEvent(const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent) const
{
    g_dispatchPointerEvent = true;
}

void JsAppImpl::DispatchAxisEvent(const std::shared_ptr<OHOS::MMI::AxisEvent>& axisEvent) const
{
    g_dispatchAxisEvent = true;
}

void JsAppImpl::DispatchInputMethodEvent(const unsigned int codePoint) const
{
    g_dispatchInputMethodEvent = true;
}

void JsAppImpl::DispatchKeyEvent(const std::shared_ptr<OHOS::MMI::KeyEvent>& keyEvent) const
{
    g_dispatchKeyEvent = true;
}