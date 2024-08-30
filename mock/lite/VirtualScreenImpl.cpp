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

#include "VirtualScreenImpl.h"

#include "draw/draw_utils.h"
#include "hal_tick.h"
#include "image_decode_ability.h"

#define boolean jpegboolean
#include "jpeglib.h"
#undef boolean
#include "task_manager.h"
#include "CommandParser.h"
#include "ModelManager.h"
#include "PreviewerEngineLog.h"
#include "TraceTool.h"

void VirtualScreenImpl::InitAll(std::string pipeName, std::string pipePort)
{
    OHOS::ImageDecodeAbility& ability = OHOS::ImageDecodeAbility::GetInstance();
    ability.SetImageDecodeAbility(OHOS::IMG_SUPPORT_BITMAP | OHOS::IMG_SUPPORT_JPEG | OHOS::IMG_SUPPORT_PNG);
    if (CommandParser::GetInstance().GetDeviceType() == "liteWearable") {
        ability.SetImageDecodeAbility(OHOS::IMG_SUPPORT_BITMAP);
    }

    InitPipe(pipeName, pipePort);
    if ((!CommandParser::GetInstance().IsResolutionValid(orignalResolutionWidth)) ||
        (!CommandParser::GetInstance().IsResolutionValid(orignalResolutionHeight))) {
        ELOG("VirtualScreen::InitAll invalid resolution, width : %d height : %d", orignalResolutionWidth,
             orignalResolutionHeight);
        return;
    }

    bufferSize = orignalResolutionWidth * orignalResolutionHeight * pixelSize + headSize;
    wholeBuffer = new(std::nothrow) uint8_t[LWS_PRE + bufferSize];
    if (!wholeBuffer) {
        ELOG("Memory allocation failed : wholeBuffer.");
        return;
    }
    regionWholeBuffer = new(std::nothrow) uint8_t[LWS_PRE + bufferSize];
    if (!regionWholeBuffer) {
        ELOG("Memory allocation failed: regionWholeBuffer.");
        return;
    }
    screenBuffer = wholeBuffer + LWS_PRE;
    regionBuffer = regionWholeBuffer + LWS_PRE;
    osBuffer = new(std::nothrow) uint8_t[bufferSize];
    if (!osBuffer) {
        ELOG("Memory allocation failed: osBuffer.");
        return;
    }
    if (screenBuffer == nullptr) {
        ELOG("VirtualScreen::InitAll wholeBuffer memory allocation failed");
        return;
    }
    InitBuffer();
}

bool VirtualScreenImpl::IsRectValid(int32_t x1, int32_t y1, int32_t x2, int32_t y2) const
{
    if (x1 < 0 || y1 < 0) {
        return false;
    }

    if (x2 >= orignalResolutionWidth || y2 >= orignalResolutionHeight) {
        return false;
    }
    return true;
}

void VirtualScreenImpl::WriteRefreshRegion()
{
    currentPos = VERSION_POS;
    WriteBuffer(protocolVersion);
    WriteBuffer(regionX1);
    WriteBuffer(regionY1);
    regionWidth = static_cast<uint16_t>(orignalResolutionWidth);
    WriteBuffer(regionWidth);
    regionHeight = static_cast<uint16_t>(orignalResolutionHeight);
    WriteBuffer(regionHeight);
}

void VirtualScreenImpl::UpdateRegion(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    regionX1 = x1;
    regionY1 = y1;
    regionX2 = (x2 < compressionResolutionWidth - extendPix)
                   ? (x2 + extendPix) : (compressionResolutionWidth - 1);
    regionY2 = (y2 < compressionResolutionHeight - extendPix)
                   ? (y2 + extendPix) : (compressionResolutionHeight - 1);
    regionWidth = regionX2 - regionX1 + 1;
    regionHeight = regionY2 - regionY1 + 1;
}

void VirtualScreenImpl::InitBuffer()
{
    currentPos = 0;
    WriteBuffer(headStart);
    WriteBuffer(orignalResolutionWidth);
    WriteBuffer(orignalResolutionHeight);
    WriteBuffer(compressionResolutionWidth);
    WriteBuffer(compressionResolutionHeight);
}

void VirtualScreenImpl::ScheduleBufferSend()
{
    if (!isChanged) {
        return;
    }

    if (!isWebSocketConfiged) {
        ELOG("image socket is not ready");
        return;
    }
    isFrameUpdated = true;
    if (CommandParser::GetInstance().IsRegionRefresh()) {
        SendFullBuffer();
    } else {
        SendFullBuffer();
    }
    if (isFirstSend) {
        ILOG("Send first buffer finish");
        TraceTool::GetInstance().HandleTrace("Send first buffer finish");
        isFirstSend = false;
    }
    
    {
        std::lock_guard<std::mutex> guard(WebSocketServer::GetInstance().mutex);
        if (!WebSocketServer::GetInstance().firstImageBuffer) {
            WebSocketServer::GetInstance().firstImageBuffer = new(std::nothrow) uint8_t[LWS_PRE + bufferSize];
            if (!WebSocketServer::GetInstance().firstImageBuffer) {
                ELOG("Memory allocation failed: firstImageBuffer.");
                return;
            }
            WebSocketServer::GetInstance().firstImagebufferSize = headSize + jpgBufferSize;
        }
        std::copy(regionBuffer,
                  regionBuffer + headSize + jpgBufferSize,
                  WebSocketServer::GetInstance().firstImageBuffer + LWS_PRE);
    }

    sendFrameCountPerMinute++;
    isChanged = false;
}

void VirtualScreenImpl::Send(unsigned char* data, int32_t width, int32_t height)
{
    if (CommandParser::GetInstance().GetScreenMode() == CommandParser::ScreenMode::STATIC
        && VirtualScreen::isOutOfSeconds) {
        return;
    }
    // if websocket is config, use websocet, else use localsocket
    VirtualScreen::RgbToJpg(data + headSize, width, height);
    std::copy(jpgScreenBuffer, jpgScreenBuffer + jpgBufferSize, regionBuffer + headSize);
    WebSocketServer::GetInstance().WriteData(regionBuffer, headSize + jpgBufferSize);
    FreeJpgMemory();
}

void VirtualScreenImpl::SendFullBuffer()
{
    WriteRefreshRegion();
    std::copy(screenBuffer, screenBuffer + headSize, regionBuffer);
    Send(reinterpret_cast<unsigned char*>(screenBuffer),
         compressionResolutionWidth,
         compressionResolutionHeight);
}

void VirtualScreenImpl::SendRegionBuffer()
{
    WriteRefreshRegion();
    std::copy(screenBuffer, screenBuffer + headSize, regionBuffer);
    for (int i = regionY1; i <= regionY2; ++i) {
        uint8_t* startPos = screenBuffer + (i * compressionResolutionWidth + regionX1) * jpgPix + headSize;
        std::copy(startPos,
                  startPos + regionWidth * jpgPix,
                  regionBuffer + ((i - regionY1) * regionWidth) * jpgPix + headSize);
    }
    Send(reinterpret_cast<unsigned char*>(regionBuffer), regionWidth, regionHeight);
}

void VirtualScreenImpl::FreeJpgMemory()
{
    if (jpgScreenBuffer != nullptr) {
        free(jpgScreenBuffer);
        jpgScreenBuffer = NULL;
    }
}

VirtualScreenImpl& VirtualScreenImpl::GetInstance()
{
    static VirtualScreenImpl virtualScreen;
    BaseGfxEngine::InitGfxEngine(&virtualScreen);
    return virtualScreen;
}

void VirtualScreenImpl::CheckBufferSend()
{
    VirtualScreenImpl::GetInstance().ScheduleBufferSend();
}

VirtualScreenImpl::VirtualScreenImpl()
    : wholeBuffer(nullptr),
      regionWholeBuffer(nullptr),
      screenBuffer(nullptr),
      regionBuffer(nullptr),
      osBuffer(nullptr),
      isChanged(false),
      currentPos(0),
      bufferSize(0),
      isFirstRender(true),
      isFirstSend(true),
      regionX1(0),
      regionY1(0),
      regionX2(0),
      regionY2(0),
      regionWidth(0),
      regionHeight(0),
      bufferInfo(nullptr)
{
}

VirtualScreenImpl::~VirtualScreenImpl()
{
    if (wholeBuffer != nullptr) {
        delete [] wholeBuffer;
        wholeBuffer = nullptr;
        screenBuffer = nullptr;
    }
    FreeJpgMemory();
    if (WebSocketServer::GetInstance().firstImageBuffer) {
        delete [] WebSocketServer::GetInstance().firstImageBuffer;
        WebSocketServer::GetInstance().firstImageBuffer = nullptr;
    }
}

void VirtualScreenImpl::Flush(const OHOS::Rect& flushRect)
{
    if (isFirstRender) {
        ILOG("Get first render buffer");
        TraceTool::GetInstance().HandleTrace("Get first render buffer");
        isFirstRender = false;
    }

    bool staticRet = VirtualScreen::JudgeStaticImage(SEND_IMG_DURATION_MS);
    if (!staticRet) {
        return;
    }

    for (int i = 0; i <= compressionResolutionHeight - 1; ++i) {
        for (int j = 0; j <= compressionResolutionWidth - 1; ++j) {
            uint8_t* curPixel = screenBuffer + (i * compressionResolutionWidth + j) * jpgPix + headSize;
            uint8_t* osPixel = osBuffer + (i * compressionResolutionWidth + j) * pixelSize + headSize;
            *(curPixel + redPos) = *(osPixel + bluePos);
            *(curPixel + greenPos) = *(osPixel + greenPos);
            *(curPixel + bluePos) = *(osPixel + redPos);
        }
    }

    validFrameCountPerMinute++;
    isChanged = true;
    ScheduleBufferSend();
}

OHOS::BufferInfo* VirtualScreenImpl::GetFBBufferInfo()
{
    if (bufferInfo == nullptr) {
        bufferInfo = new(std::nothrow) OHOS::BufferInfo;
        if (!bufferInfo) {
            ELOG("Memory allocation failed: osBuffer.");
            return bufferInfo;
        }
        bufferInfo->rect = {0, 0, compressionResolutionWidth - 1, compressionResolutionHeight - 1};
        bufferInfo->mode = OHOS::ARGB8888;

        bufferInfo->color = 0x44;
        bufferInfo->phyAddr = bufferInfo->virAddr = osBuffer + headSize;
        // 3: Shift right 3 bits
        bufferInfo->stride = orignalResolutionWidth * (OHOS::DrawUtils::GetPxSizeByColorMode(bufferInfo->mode) >> 3);
        bufferInfo->width = orignalResolutionWidth;
        bufferInfo->height = orignalResolutionHeight;
    }

    return bufferInfo;
}

uint16_t VirtualScreenImpl::GetScreenWidth()
{
    return orignalResolutionWidth;
}

uint16_t VirtualScreenImpl::GetScreenHeight()
{
    return orignalResolutionHeight;
}
