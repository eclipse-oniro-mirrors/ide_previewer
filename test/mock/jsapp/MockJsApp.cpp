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
    return deviceType == "liteWearable" || deviceType == "smartVersion";
}

std::string JsApp::GetJSONTree() {
    //Only for mock test, no specific implementation
}
std::string JsApp::GetDefaultJSONTree() {
    //Only for mock test, no specific implementation
}
void JsApp::OrientationChanged(std::string commandOrientation) {
    //Only for mock test, no specific implementation
}
void JsApp::ResolutionChanged(ResolutionParam& param, int32_t screenDensity, std::string reason) {
    //Only for mock test, no specific implementation
}
void JsApp::SetArgsColorMode(const std::string& value) {
    //Only for mock test, no specific implementation
}
void JsApp::SetArgsAceVersion(const std::string& value) {
    //Only for mock test, no specific implementation
}
std::string JsApp::GetOrientation() const {
    //Only for mock test, no specific implementation
}
std::string JsApp::GetColorMode() const {
    //Only for mock test, no specific implementation
}
void JsApp::ColorModeChanged(const std::string commandColorMode) {
    //Only for mock test, no specific implementation
}
void JsApp::ReloadRuntimePage(const std::string) {
    //Only for mock test, no specific implementation
}
void JsApp::SetScreenDensity(const std::string value) {
    //Only for mock test, no specific implementation
}
void JsApp::SetConfigChanges(const std::string value) {
    //Only for mock test, no specific implementation
}
bool JsApp::MemoryRefresh(const std::string) const {
    //Only for mock test, no specific implementation
}
void JsApp::LoadDocument(const std::string, const std::string, const Json2::Value&) {
    //Only for mock test, no specific implementation
}
void JsApp::FoldStatusChanged(const std::string commandFoldStatus, int32_t width, int32_t height) {
    //Only for mock test, no specific implementation
}
void JsApp::SetAvoidArea(const AvoidAreas& areas) {
    //Only for mock test, no specific implementation
}
void JsApp::Stop() {
    //Only for mock test, no specific implementation
}
void JsApp::InitJsApp() {
    //Only for mock test, no specific implementation
}
const AvoidAreas JsApp::GetCurrentAvoidArea() const
{
    AvoidAreas areas;
    return areas;
}