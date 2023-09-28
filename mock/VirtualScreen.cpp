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

#include "VirtualScreen.h"
#include "CommandParser.h"
#include "CppTimerManager.h"
#include "PreviewerEngineLog.h"
#include "VirtualScreen.h"

#define boolean jpegboolean
#include "jpeglib.h"
#undef boolean

using namespace std;

uint32_t VirtualScreen::validFrameCountPerMinute = 0;
uint32_t VirtualScreen::invalidFrameCountPerMinute = 0;
uint32_t VirtualScreen::sendFrameCountPerMinute = 0;
uint32_t VirtualScreen::inputKeyCountPerMinute = 0;
uint32_t VirtualScreen::inputMethodCountPerMinute = 0;
bool VirtualScreen::isWebSocketListening = false;
std::string VirtualScreen::webSocketPort = "";

std::chrono::system_clock::time_point VirtualScreen::startTime = std::chrono::system_clock::now();
std::chrono::system_clock::time_point VirtualScreen::staticCardStartTime = std::chrono::system_clock::now();
bool VirtualScreen::isStartCount = true;
bool VirtualScreen::isOutOfSeconds = false;

VirtualScreen::VirtualScreen()
    : isFrameUpdated(false),
      orignalResolutionWidth(0),
      orignalResolutionHeight(0),
      compressionResolutionWidth(0),
      compressionResolutionHeight(0),
      screenSocket(nullptr),
      frameCountTimer(nullptr),
      isWebSocketConfiged(false),
      currentRouter(""),
      jpgScreenBuffer(nullptr),
      jpgBufferSize(0)
{
}

VirtualScreen::~VirtualScreen()
{
    if (screenSocket != nullptr) {
        screenSocket->DisconnectFromServer();
        delete screenSocket;
        screenSocket = nullptr;
    }
}

std::string VirtualScreen::GetCurrentRouter() const
{
    return currentRouter;
}

void VirtualScreen::SetCurrentRouter(const std::string currentRouterValue)
{
    currentRouter = currentRouterValue;
}

std::string VirtualScreen::GetAbilityCurrentRouter() const
{
    return abilityCurrentRouter;
}

void VirtualScreen::SetAbilityCurrentRouter(const std::string currentRouterValue)
{
    abilityCurrentRouter = currentRouterValue;
}

int32_t VirtualScreen::GetOrignalWidth() const
{
    return orignalResolutionWidth;
}

void VirtualScreen::SetOrignalWidth(const int32_t& value)
{
    orignalResolutionWidth = value;
}

int32_t VirtualScreen::GetOrignalHeight() const
{
    return orignalResolutionHeight;
}

void VirtualScreen::SetOrignalHeight(const int32_t& value)
{
    orignalResolutionHeight = value;
}

int32_t VirtualScreen::GetCompressionWidth() const
{
    return compressionResolutionWidth;
}

void VirtualScreen::SetCompressionWidth(const int32_t& value)
{
    compressionResolutionWidth = value;
}

int32_t VirtualScreen::GetCompressionHeight() const
{
    return compressionResolutionHeight;
}

void VirtualScreen::SetCompressionHeight(const int32_t& value)
{
    compressionResolutionHeight = value;
}

void VirtualScreen::InitPipe(string pipeName, string pipePort)
{
    webSocketPort = pipePort;
    isWebSocketConfiged = true;
    WebSocketServer::GetInstance().SetServerPort(atoi(pipePort.c_str()));
    WebSocketServer::GetInstance().Run();
    isWebSocketListening = true;
}

void VirtualScreen::InitVirtualScreen()
{
    int32_t orignalWidth = CommandParser::GetInstance().GetOrignalResolutionWidth();
    int32_t orignalHeight = CommandParser::GetInstance().GetOrignalResolutionHeight();
    int32_t compressionWidth = CommandParser::GetInstance().GetCompressionResolutionWidth();
    int32_t compressionHeight = CommandParser::GetInstance().GetCompressionResolutionHeight();
    SetVirtualScreenWidthAndHeight(orignalWidth, orignalHeight, compressionWidth, compressionHeight);
}

void VirtualScreen::SetVirtualScreenWidthAndHeight(const int32_t& orignalWidth,
                                                   const int32_t& orignalHeight,
                                                   const int32_t& compressionWidth,
                                                   const int32_t& compressionHeight)
{
    VirtualScreen::SetOrignalWidth(orignalWidth);
    VirtualScreen::SetOrignalHeight(orignalHeight);
    VirtualScreen::SetCompressionWidth(compressionWidth);
    VirtualScreen::SetCompressionHeight(compressionHeight);
}

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

void VirtualScreen::InitFrameCountTimer()
{
    if (frameCountTimer.get() != nullptr) {
        ILOG("VirtualScreen::InitFrameCountTimer timer is already started.");
        return;
    }

    frameCountTimer = make_unique<CppTimer>(VirtualScreen::PrintFrameCount);
    if (frameCountTimer == nullptr) {
        ELOG("JsApp::InitTimer taskHandleTimer memory allocation failed.");
        return;
    }
    CppTimerManager::GetTimerManager().AddCppTimer(*frameCountTimer);
    frameCountTimer->Start(frameCountPeriod);
}

void VirtualScreen::PrintFrameCount()
{
    if ((validFrameCountPerMinute | invalidFrameCountPerMinute | sendFrameCountPerMinute |
        inputKeyCountPerMinute | inputMethodCountPerMinute) == 0) {
        return;
    }

    ELOG("ValidFrameCount: %d InvalidFrameCount: %d SendFrameCount: %d inputKeyCount: %d\
         inputMethodCount: %d", validFrameCountPerMinute, invalidFrameCountPerMinute,
         sendFrameCountPerMinute, inputKeyCountPerMinute, inputMethodCountPerMinute);
    validFrameCountPerMinute = 0;
    invalidFrameCountPerMinute = 0;
    sendFrameCountPerMinute = 0;
    inputKeyCountPerMinute = 0;
    inputMethodCountPerMinute = 0;
}


void VirtualScreen::SetLoadDocFlag(VirtualScreen::LoadDocType flag)
{
    startLoadDoc = flag;
}

VirtualScreen::LoadDocType VirtualScreen::GetLoadDocFlag() const
{
    return startLoadDoc;
}

int VirtualScreen::GetJpgQualityValue(int32_t width, int32_t height) const
{
    long long pixCount = static_cast<long long>(width) * static_cast<long long>(height);
    if (pixCount <= static_cast<int>(JpgPixCountLevel::LOWCOUNT)) {
        return static_cast<int>(JpgQualityLevel::HIGHLEVEL);
    } else if (pixCount > static_cast<int>(JpgPixCountLevel::LOWCOUNT) &&
        pixCount <= static_cast<int>(JpgPixCountLevel::MIDDLECOUNT)) {
        return static_cast<int>(JpgQualityLevel::MIDDLELEVEL);
    } else if (pixCount > static_cast<int>(JpgPixCountLevel::MIDDLECOUNT) &&
        pixCount <= static_cast<int>(JpgPixCountLevel::HIGHCOUNT)) {
        return static_cast<int>(JpgQualityLevel::LOWLEVEL);
    } else {
        return static_cast<int>(JpgQualityLevel::DEFAULTLEVEL);
    }
}

std::string VirtualScreen::GetFastPreviewMsg() const
{
    return fastPreviewMsg;
}

void VirtualScreen::SetFastPreviewMsg(const std::string msg)
{
    fastPreviewMsg = msg;
}

void VirtualScreen::SetDropFrameFrequency(const int32_t& value)
{
    dropFrameFrequency = value;
    startDropFrameTime = std::chrono::system_clock::now();
}

bool VirtualScreen::JudgeAndDropFrame()
{
    if (dropFrameFrequency <= 0) {
        return false;
    }
    auto endTime = std::chrono::system_clock::now();
    int64_t timePassed = chrono::duration_cast<chrono::milliseconds>(endTime -
                        startDropFrameTime).count();
    if (timePassed >= dropFrameFrequency) {
        startDropFrameTime = std::chrono::system_clock::now();
    }
    return timePassed < dropFrameFrequency;
}

bool VirtualScreen::JudgeStaticImage(const int duration)
{
    if (CommandParser::GetInstance().GetScreenMode() == CommandParser::ScreenMode::STATIC) {
        if (VirtualScreen::isOutOfSeconds) {
            return false;
        }
        if (VirtualScreen::isStartCount) {
            VirtualScreen::isStartCount = false;
            VirtualScreen::startTime = std::chrono::system_clock::now();
        }
        auto endTime = std::chrono::system_clock::now();
        int64_t timePassed = chrono::duration_cast<chrono::milliseconds>(endTime -
                             VirtualScreen::startTime).count();
        if (timePassed > duration) {
            VirtualScreen::isOutOfSeconds = true;
            return false;
        }
    }
    return true;
}

bool VirtualScreen::StopSendStaticCardImage(const int duration)
{
    if (CommandParser::GetInstance().IsStaticCard()) {
        static bool first = true;
        if (first) {
            first = false;
            VirtualScreen::staticCardStartTime = std::chrono::system_clock::now();
        }
        auto endTime = std::chrono::system_clock::now();
        int64_t timePassed = chrono::duration_cast<chrono::milliseconds>(endTime -
                             VirtualScreen::staticCardStartTime).count();
        if (timePassed > duration) {
            return true;
        }
    }
    return false;
}

void VirtualScreen::RgbToJpg(unsigned char* data, const int32_t width, const int32_t height)
{
    if (width < 1 || height < 1) {
        FLOG("VirtualScreenImpl::RgbToJpg the width or height is invalid value");
    }
    jpeg_compress_struct jpeg = {0};
    jpeg_error_mgr jerr;
    jpeg.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&jpeg);
    jpeg_mem_dest(&jpeg, &jpgScreenBuffer, &jpgBufferSize);
    jpeg.image_width = width;
    jpeg.image_height = height;
    jpeg.input_components = jpgPix;
    jpeg.in_color_space = JCS_RGB;
    jpeg_set_defaults(&jpeg);
    jpeg_set_quality(&jpeg, GetJpgQualityValue(width, height), TRUE);
    jpeg_start_compress(&jpeg, TRUE);
    JSAMPROW rowPointer[1];
    int rowStride = width * jpgPix;
    while (jpeg.next_scanline < jpeg.image_height) {
        rowPointer[0] = &data[jpeg.next_scanline * rowStride];
        jpeg_write_scanlines(&jpeg, rowPointer, 1);
    }
    jpeg_finish_compress(&jpeg);
    jpeg_destroy_compress(&jpeg);
}