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

#include "VirtualScreen.h"
#include "MockGlobalResult.h"

using namespace std;

VirtualScreen::VirtualScreen() {}
VirtualScreen::~VirtualScreen() {}

uint32_t VirtualScreen::inputKeyCountPerMinute = 0;
uint32_t VirtualScreen::inputMethodCountPerMinute = 0;
bool VirtualScreen::isWebSocketListening = false;
std::string VirtualScreen::webSocketPort = "";

void VirtualScreen::InitFlushEmptyTime() {}

int32_t VirtualScreen::GetCurrentWidth() const
{
    g_getCurrentWidth = true;
    return 1080; // 1080 is screen default width
}

int32_t VirtualScreen::GetCurrentHeight() const
{
    g_getCurrentHeight = true;
    return 2340; // 2340 is screen default height
}

std::string VirtualScreen::GetCurrentRouter() const
{
    g_getCurrentRouter = true;
    return "pages/Index";
}

std::string VirtualScreen::GetAbilityCurrentRouter() const
{
    g_getAbilityCurrentRouter = true;
    return "pages/Index";
}

std::string VirtualScreen::GetFastPreviewMsg() const
{
    g_getFastPreviewMsg = true;
    return "getFastPreviewMsg";
}

void VirtualScreen::SetDropFrameFrequency(const int32_t& value)
{
    dropFrameFrequency = value;
}

std::string VirtualScreen::GetFoldStatus() const
{
    g_getFoldStatus = true;
    return foldStatus;
}

void VirtualScreen::SetFoldStatus(const std::string& value)
{
    foldStatus = value;
}

void VirtualScreen::SetLoadDocFlag(VirtualScreen::LoadDocType flag)
{
    startLoadDoc = flag;
}

void VirtualScreen::InitVirtualScreen()
{
    orignalResolutionWidth = 1080; // 1080 is default width
    orignalResolutionHeight = 2340; // 2340 is default height
    compressionResolutionWidth = 1080; // 1080 is default width
    compressionResolutionHeight = 2340; // 2340 is default height
}

void VirtualScreen::InitPipe(string pipeName, string pipePort) {}

void VirtualScreen::WidthAndHeightReverse()
{
    int32_t temp = 0;
    temp = orignalResolutionHeight;
    orignalResolutionHeight = orignalResolutionWidth;
    orignalResolutionWidth = temp;
    temp = compressionResolutionHeight;
    compressionResolutionHeight = compressionResolutionWidth;
    compressionResolutionWidth = temp;
}

bool VirtualScreen::GetFoldable() const
{
    return foldable;
}

int32_t VirtualScreen::GetFoldWidth() const
{
    return foldWidth;
}

int32_t VirtualScreen::GetFoldHeight() const
{
    return foldHeight;
}

int32_t VirtualScreen::GetOrignalWidth() const
{
    return orignalResolutionWidth;
}

int32_t VirtualScreen::GetOrignalHeight() const
{
    return orignalResolutionHeight;
}

int32_t VirtualScreen::GetCompressionWidth() const
{
    return compressionResolutionWidth;
}

int32_t VirtualScreen::GetCompressionHeight() const
{
    return compressionResolutionHeight;
}

void VirtualScreen::SetCurrentResolution(int32_t width, int32_t height)
{
    currentWidth = width;
    currentHeight = height;
}