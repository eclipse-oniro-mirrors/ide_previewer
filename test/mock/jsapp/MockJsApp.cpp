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

#include "JsApp.h"

JsApp::JsApp() {}

bool JsApp::IsLiteDevice(std::string deviceType)
{
    return (deviceType == "liteWearable" || deviceType == "smartVersion") ? true : false;
}

std::string JsApp::GetJSONTree() {}
std::string JsApp::GetDefaultJSONTree() {}
void JsApp::OrientationChanged(std::string commandOrientation) {}
void JsApp::ResolutionChanged(ResolutionParam& param, int32_t screenDensity, std::string reason) {}
void JsApp::SetArgsColorMode(const std::string& value) {}
void JsApp::SetArgsAceVersion(const std::string& value) {}
std::string JsApp::GetOrientation() const {}
std::string JsApp::GetColorMode() const {}
void JsApp::ColorModeChanged(const std::string commandColorMode) {}
void JsApp::ReloadRuntimePage(const std::string) {}
void JsApp::SetScreenDensity(const std::string value) {}
void JsApp::SetConfigChanges(const std::string value) {}
bool JsApp::MemoryRefresh(const std::string) const {}
void JsApp::LoadDocument(const std::string, const std::string, const Json2::Value&) {}
void JsApp::FoldStatusChanged(const std::string commandFoldStatus, int32_t width, int32_t height) {}