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

#define boolean jpegboolean
#include "jpeglib.h"
#undef boolean

#include "CommandLineInterface.h"
#include "CommandParser.h"
#include "PreviewerEngineLog.h"
#include "TraceTool.h"

using namespace std;

VirtualScreenImpl& VirtualScreenImpl::GetInstance()
{
    static VirtualScreenImpl virtualScreen;
    return virtualScreen;
}

void VirtualScreenImpl::StartTimer()
{
    while (true) {
        auto endTime = std::chrono::system_clock::now();
        int64_t timePassed = chrono::duration_cast<chrono::milliseconds>(endTime -
                             VirtualScreenImpl::GetInstance().startTime).count();
        if (timePassed >= SEND_IMG_DURATION_MS) {
            GetInstance().SetLoadDocFlag(VirtualScreen::LoadDocType::NORMAL);
            VirtualScreen::isStartCount = true;
            {
                std::lock_guard<std::mutex> guard(WebSocketServer::GetInstance().mutex);
                if (GetInstance().loadDocCopyBuffer != nullptr) {
                    delete [] GetInstance().loadDocCopyBuffer;
                    GetInstance().loadDocCopyBuffer = nullptr;
                }
                GetInstance().loadDocCopyBuffer = new uint8_t[GetInstance().lengthTemp];
                std::copy(GetInstance().loadDocTempBuffer,
                          GetInstance().loadDocTempBuffer + GetInstance().lengthTemp,
                          GetInstance().loadDocCopyBuffer);
            }
            VirtualScreenImpl::GetInstance().protocolVersion =
                static_cast<uint16_t>(VirtualScreen::ProtocolVersion::LOADDOC);
            GetInstance().bufferSize = GetInstance().lengthTemp + GetInstance().headSize;
            GetInstance().wholeBuffer = new uint8_t[LWS_PRE + GetInstance().bufferSize];
            GetInstance().screenBuffer = GetInstance().wholeBuffer + LWS_PRE;
            GetInstance().SendPixmap(GetInstance().loadDocCopyBuffer,
                                     GetInstance().lengthTemp,
                                     GetInstance().widthTemp,
                                     GetInstance().heightTemp);
            ILOG("LoadDocFlag2:finished");
            return;
        }
    }
}

bool VirtualScreenImpl::LoadDocCallback(const void* data,
                                        const size_t length,
                                        const int32_t width,
                                        const int32_t height)
{
    if (GetInstance().GetLoadDocFlag() == VirtualScreen::LoadDocType::FINISHED) {
        {
            std::lock_guard<std::mutex> guard(WebSocketServer::GetInstance().mutex);
            if (GetInstance().loadDocTempBuffer != nullptr) {
                delete [] GetInstance().loadDocTempBuffer;
                GetInstance().loadDocTempBuffer = nullptr;
            }
            GetInstance().lengthTemp = length;
            GetInstance().widthTemp = width;
            GetInstance().heightTemp = height;
            if (length <= 0) {
                return false;
            }
            GetInstance().loadDocTempBuffer = new uint8_t[length];
            uint8_t*  dataPtr = reinterpret_cast<uint8_t*>(const_cast<void*>(data));
            std::copy(dataPtr, dataPtr + length, GetInstance().loadDocTempBuffer);
        }
        if (VirtualScreen::isStartCount) {
            VirtualScreen::isStartCount = false;
            VirtualScreen::startTime = std::chrono::system_clock::now();
            thread timerThread(std::ref(VirtualScreenImpl::StartTimer));
            timerThread.detach();
        }
        return false;
    }
    return true;
}

bool VirtualScreenImpl::CallBack(const void* data, const size_t length,
                                 const int32_t width, const int32_t height)
{
    if (VirtualScreenImpl::GetInstance().StopSendStaticCardImage(STOP_SEND_CARD_DURATION_MS)) {
        return false;
    }
    if (VirtualScreenImpl::GetInstance().GetLoadDocFlag() < VirtualScreen::LoadDocType::FINISHED) {
        return false;
    }
    if (VirtualScreenImpl::GetInstance().JudgeAndDropFrame()) {
        return false;
    }

    bool staticRet = VirtualScreen::JudgeStaticImage(SEND_IMG_DURATION_MS);
    if (!staticRet) {
        return false;
    }

    if (!LoadDocCallback(data, length, width, height)) {
        return false;
    }

    GetInstance().bufferSize = length + GetInstance().headSize;
    GetInstance().wholeBuffer = new uint8_t[LWS_PRE + GetInstance().bufferSize];
    GetInstance().screenBuffer = GetInstance().wholeBuffer + LWS_PRE;

    return GetInstance().SendPixmap(data, length, width, height);
}

bool VirtualScreenImpl::PageCallBack(const std::string currentRouterPath)
{
    std::string currentRouter = currentRouterPath.substr(0, currentRouterPath.size() - 3);
    ILOG("PageCallback currentPage is : %s", currentRouter.c_str());
    GetInstance().SetCurrentRouter(currentRouter);
    Json::Value val;
    CommandLineInterface::GetInstance().CreatCommandToSendData("CurrentRouter", val, "get");
    return true;
}

bool VirtualScreenImpl::LoadContentCallBack(const std::string currentRouterPath)
{
    ILOG("LoadContentCallback currentPage is : %s", currentRouterPath.c_str());
    GetInstance().SetAbilityCurrentRouter(currentRouterPath);
    Json::Value val;
    CommandLineInterface::GetInstance().CreatCommandToSendData("LoadContent", val, "get");
    return true;
}

void VirtualScreenImpl::FastPreviewCallBack(const std::string& jsonStr)
{
    GetInstance().SetFastPreviewMsg(jsonStr);
    Json::Value val;
    CommandLineInterface::GetInstance().CreatCommandToSendData("FastPreviewMsg", val, "get");
}

void VirtualScreenImpl::InitAll(string pipeName, string pipePort)
{
    VirtualScreen::InitPipe(pipeName, pipePort);
}

VirtualScreenImpl::VirtualScreenImpl()
    : isFirstSend(true),
      isFirstRender(true),
      writed(0),
      wholeBuffer(nullptr),
      screenBuffer(nullptr),
      bufferSize(0),
      currentPos(0)
{
}

VirtualScreenImpl::~VirtualScreenImpl()
{
    FreeJpgMemory();
    if (WebSocketServer::GetInstance().firstImageBuffer) {
        delete [] WebSocketServer::GetInstance().firstImageBuffer;
        WebSocketServer::GetInstance().firstImageBuffer = nullptr;
    }
    if (VirtualScreenImpl::GetInstance().loadDocTempBuffer != nullptr) {
        delete [] VirtualScreenImpl::GetInstance().loadDocTempBuffer;
        VirtualScreenImpl::GetInstance().loadDocTempBuffer = nullptr;
    }
    if (VirtualScreenImpl::GetInstance().loadDocCopyBuffer != nullptr) {
        delete [] VirtualScreenImpl::GetInstance().loadDocCopyBuffer;
        VirtualScreenImpl::GetInstance().loadDocCopyBuffer = nullptr;
    }
}

void VirtualScreenImpl::Send(const void* data, int32_t retWidth, int32_t retHeight)
{
    if (CommandParser::GetInstance().GetScreenMode() == CommandParser::ScreenMode::STATIC
        && VirtualScreen::isOutOfSeconds) {
        return;
    }

    if (retWidth < 1 || retHeight < 1) {
        FLOG("VirtualScreenImpl::RgbToJpg the retWidth or height is invalid value");
    }
    unsigned char* dataTemp = new unsigned char[retWidth * retHeight * jpgPix];
    for (int i = 0; i < retHeight; i++) {
        for (int j = 0; j < retWidth; j++) {
            int input_base_pos = i * retWidth * pixelSize + j * pixelSize;
            int now_base_pos = i * retWidth * jpgPix + j * jpgPix;
            dataTemp[now_base_pos + redPos] = *((char*)data + input_base_pos + redPos);
            dataTemp[now_base_pos + greenPos] = *((char*)data + input_base_pos + greenPos);
            dataTemp[now_base_pos + bluePos] = *((char*)data + input_base_pos + bluePos);
        }
    }
    VirtualScreen::RgbToJpg(dataTemp, retWidth, retHeight);
    delete [] dataTemp;
    if (jpgBufferSize > bufferSize - headSize) {
        FLOG("VirtualScreenImpl::Send length must < %d", bufferSize - headSize);
    }

    std::copy(jpgScreenBuffer, jpgScreenBuffer + jpgBufferSize, screenBuffer + headSize);
    writed = WebSocketServer::GetInstance().WriteData(screenBuffer, headSize + jpgBufferSize);
    std::lock_guard<std::mutex> guard(WebSocketServer::GetInstance().mutex);
    if (WebSocketServer::GetInstance().firstImageBuffer) {
        delete [] WebSocketServer::GetInstance().firstImageBuffer;
        WebSocketServer::GetInstance().firstImageBuffer = nullptr;
    }
    WebSocketServer::GetInstance().firstImageBuffer = new uint8_t[LWS_PRE + bufferSize];
    WebSocketServer::GetInstance().firstImagebufferSize = headSize + jpgBufferSize;
    std::copy(screenBuffer,
              screenBuffer + headSize + jpgBufferSize,
              WebSocketServer::GetInstance().firstImageBuffer + LWS_PRE);

    FreeJpgMemory();
}

bool VirtualScreenImpl::SendPixmap(const void* data, size_t length, int32_t retWidth, int32_t retHeight)
{
    if (data == nullptr) {
        ELOG("render callback data is null.");
        invalidFrameCountPerMinute++;
        return false;
    }

    if (!isWebSocketConfiged) {
        ELOG("image socket is not ready");
        return false;
    }

    if (isFirstRender) {
        ILOG("Get first render buffer");
        TraceTool::GetInstance().HandleTrace("Get first render buffer");
        isFirstRender = false;
    }

    isFrameUpdated = true;
    currentPos = 0;

    WriteBuffer(headStart);
    WriteBuffer(retWidth);
    WriteBuffer(retHeight);
    WriteBuffer(retWidth);
    WriteBuffer(retHeight);
    if (!CommandParser::GetInstance().IsRegionRefresh()) {
        for (size_t i = 0; i < headReservedSize / sizeof(int32_t); i++) {
            WriteBuffer(static_cast<uint32_t>(0));
        }
    } else {
        uint16_t x1 = 0;
        uint16_t y1 = 0;
        uint16_t width = static_cast<uint16_t>(retWidth);
        uint16_t height = static_cast<uint16_t>(retHeight);
        WriteBuffer(protocolVersion);
        WriteBuffer(x1);
        WriteBuffer(y1);
        WriteBuffer(width);
        WriteBuffer(height);
        for (size_t i = 0; i < 10 / sizeof(uint16_t); i++) { // fill 10bytes for header
            WriteBuffer(static_cast<uint16_t>(0));
        }
    }
    Send(data, retWidth, retHeight);
    if (isFirstSend) {
        ILOG("Send first buffer finish");
        TraceTool::GetInstance().HandleTrace("Send first buffer finish");
        isFirstSend = false;
    }

    validFrameCountPerMinute++;
    sendFrameCountPerMinute++;
    return writed == length;
}

void VirtualScreenImpl::FreeJpgMemory()
{
    if (wholeBuffer != nullptr) {
        delete [] wholeBuffer;
        wholeBuffer = nullptr;
        screenBuffer = nullptr;
    }
    if (jpgScreenBuffer != nullptr) {
        free(jpgScreenBuffer);
        jpgScreenBuffer = NULL;
        jpgBufferSize = 0;
    }
    if (VirtualScreenImpl::GetInstance().loadDocCopyBuffer != nullptr) {
        delete [] VirtualScreenImpl::GetInstance().loadDocCopyBuffer;
        VirtualScreenImpl::GetInstance().loadDocCopyBuffer = nullptr;
    }
}
