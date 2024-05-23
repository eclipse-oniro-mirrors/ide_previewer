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

#define private public
#include "ace_ability.h"
#include "window.h"
#include "MockGlobalResult.h"

namespace OHOS::Ace::Platform {
AceAbility::AceAbility(const AceRunArgs& runArgs) : runArgs_(runArgs) {}

AceAbility::~AceAbility() {}

void AceAbility::SetMockModuleList(const std::map<std::string, std::string>& mockJsonInfo)
{
    g_setMockModuleList = true;
}

void AceAbility::InitEnv()
{
    g_initEnv = true;
}

void AceAbility::OnBackPressed() const
{
    g_onBackPressed = true;
}

bool AceAbility::OnInputEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) const
{
    g_onInputEvent = true;
    return true;
}
bool AceAbility::OnInputEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const
{
    g_onInputEvent = true;
    return true;
}
bool AceAbility::OnInputEvent(const std::shared_ptr<MMI::AxisEvent>& axisEvent) const
{
    g_onInputEvent = true;
    return true;
}
bool AceAbility::OnInputMethodEvent(const unsigned int codePoint) const
{
    g_onInputMethodEvent = true;
    return true;
}

void AceAbility::OnConfigurationChanged(const DeviceConfig& newConfig)
{
    g_onConfigurationChanged = true;
}

void AceAbility::SurfaceChanged(
    const DeviceOrientation& orientation, const double& resolution, int32_t& width, int32_t& height,
    WindowSizeChangeReason type)
{
    g_surfaceChanged = true;
}

void AceAbility::ReplacePage(const std::string& url, const std::string& params)
{
    g_replacePage = true;
}
void AceAbility::LoadDocument(const std::string& url, const std::string& componentName, SystemParams& systemParams)
{
    g_loadAceDocument = true;
}

std::string AceAbility::GetJSONTree()
{
    return "jsontree";
}

std::string AceAbility::GetDefaultJSONTree()
{
    return "defaultjsontree";
}

bool AceAbility::OperateComponent(const std::string& attrsJson)
{
    g_operateComponent = true;
    return true;
}

void AceAbility::SetWindow(sptr<OHOS::Rosen::Window> rsWindow)
{
    rsWindow_ = rsWindow;
    g_setWindow = true;
}

std::unique_ptr<AceAbility> AceAbility::CreateInstance(AceRunArgs& runArgs)
{
    auto aceAbility = std::make_unique<AceAbility>(runArgs);
    return aceAbility;
}

sptr<OHOS::Rosen::Window> AceAbility::GetWindow()
{
    return rsWindow_;
}
}