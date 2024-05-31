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

#include <cinttypes>
#define boolean jpegboolean
#include "jpeglib.h"
#undef boolean

#include "CommandLineInterface.h"
#include "CommandParser.h"
#include "PreviewerEngineLog.h"
#include "TraceTool.h"
#include <sstream>

using namespace std;

VirtualScreenImpl& VirtualScreenImpl::GetInstance()
{
    static VirtualScreenImpl virtualScreen;
    return virtualScreen;
}

void VirtualScreenImpl::SendBufferOnTimer()
{
    GetInstance().SetLoadDocFlag(VirtualScreen::LoadDocType::NORMAL);
    if (GetInstance().loadDocTempBuffer == nullptr) {
        PrintLoadDocFinishedLog("onRender timeout,no buffer to send");
        return;
    }
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
    GetInstance().SendPixmap(GetInstance().loadDocCopyBuffer, GetInstance().lengthTemp,
        GetInstance().widthTemp, GetInstance().heightTemp);
}

void VirtualScreenImpl::PrintLoadDocFinishedLog(const std::string& logStr)
{
    ILOG("loadDoc: LoadDocFlag2:finished %s, onRenderTime:%" PRIu64 ", flushEmptyTime:%" PRIu64 ", \
        onRenderTimeStamp:%" PRIu64 " flushEmptyTimeStamp:%" PRIu64 "", logStr.c_str(),
        GetInstance().onRenderTime, GetInstance().flushEmptyTime,
        GetInstance().timeStampTemp, GetInstance().flushEmptyTimeStamp);
}

bool VirtualScreenImpl::FlushEmptyFunc(std::chrono::system_clock::time_point endTime, int64_t timePassed)
{
    if (GetInstance().onRenderTime > GetInstance().flushEmptyTime) {
        if (GetInstance().timeStampTemp < GetInstance().flushEmptyTimeStamp) {
            SendBufferOnTimer(); // 有收到结束标记，且flushEmpty后有继续出图
            PrintLoadDocFinishedLog("flushEmpty normal, onRender normal");
            return true;
        }
    } else {
        // flushEmpty后没有继续出图，计时100ms，如果仍没有onRender，发送上一次的的onRender buffer
        int64_t timePassed2 = chrono::duration_cast<chrono::milliseconds>(endTime -
            GetInstance().flushEmptyTime).count();
        if (timePassed2 > TIMEOUT_ONRENDER_DURATION_MS) {
            if (GetInstance().timeStampTemp < GetInstance().flushEmptyTimeStamp) {
                SendBufferOnTimer();
                PrintLoadDocFinishedLog("flushEmpty normal, onRender timeout");
                return true;
            }
        }
    }
    if (timePassed >= TIMEOUT_NINE_S) { // 有结束点，无出图
        PrintLoadDocFinishedLog("flushEmpty normal, onRender timeout");
        return true; // 有收到结束标记，且超过最大时限还没有出图则结束
    }
    return false;
}

bool VirtualScreenImpl::NoFlushEmptyFunc(int64_t timePassed)
{
    if (timePassed >= SEND_IMG_DURATION_MS &&
        GetInstance().onRenderTime != std::chrono::system_clock::time_point::min()) {
        SendBufferOnTimer(); // 没有收到结束标记，300ms内有出图选取最后一张图
        PrintLoadDocFinishedLog("async load, flushEmpty timeout, onRender normal");
        return true;
    }
    if (timePassed >= TIMEOUT_NINE_S) {
        SendBufferOnTimer();
        PrintLoadDocFinishedLog("flushEmpty timeout, onRender unknown");
        return true; // 没有收到结束标记，且超过最大时限还没有出图则结束
    }
    return false;
}


void VirtualScreenImpl::StartTimer()
{
    while (true) {
        auto endTime = std::chrono::system_clock::now();
        int64_t timePassed = chrono::duration_cast<chrono::milliseconds>(endTime -
                                VirtualScreenImpl::GetInstance().startTime).count();
        bool ret = false;
        if (GetInstance().isFlushEmpty) {
            ret = FlushEmptyFunc(endTime, timePassed);
        } else {
            ret = NoFlushEmptyFunc(timePassed);
        }
        if (ret) {
            return;
        }
    }
}

bool VirtualScreenImpl::LoadDocCallback(const void* data, const size_t length, const int32_t width,
                                        const int32_t height, const uint64_t timeStamp)
{
    if (timeStamp < GetInstance().loadDocTimeStamp) {
        return false;
    }
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
            GetInstance().timeStampTemp = timeStamp;
            if (length <= 0) {
                return false;
            }
            GetInstance().loadDocTempBuffer = new uint8_t[length];
            uint8_t*  dataPtr = reinterpret_cast<uint8_t*>(const_cast<void*>(data));
            std::copy(dataPtr, dataPtr + length, GetInstance().loadDocTempBuffer);
            GetInstance().onRenderTime = std::chrono::system_clock::now();
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

bool VirtualScreenImpl::Callback(const void* data, const size_t length,
                                 const int32_t width, const int32_t height, const uint64_t timeStamp)
{
    ILOG("loadDoc: Callback timeStamp%" PRIu64 "", timeStamp);
    if (VirtualScreenImpl::GetInstance().StopSendStaticCardImage(STOP_SEND_CARD_DURATION_MS)) {
        return false; // 静态卡片
    }
    if (VirtualScreenImpl::GetInstance().GetLoadDocFlag() < VirtualScreen::LoadDocType::FINISHED) {
        return false;
    }
    if (VirtualScreenImpl::GetInstance().JudgeAndDropFrame()) {
        return false; // 丢帧*
    }
    bool staticRet = VirtualScreen::JudgeStaticImage(SEND_IMG_DURATION_MS);
    if (!staticRet) {
        return false; // 平行世界
    }
    if (!LoadDocCallback(data, length, width, height, timeStamp)) {
        return false; // 组件预览
    }

    GetInstance().bufferSize = length + GetInstance().headSize;
    GetInstance().wholeBuffer = new uint8_t[LWS_PRE + GetInstance().bufferSize];
    GetInstance().screenBuffer = GetInstance().wholeBuffer + LWS_PRE;

    return GetInstance().SendPixmap(data, length, width, height);
}

bool VirtualScreenImpl::FlushEmptyCallback(const uint64_t timeStamp)
{
    if (timeStamp < GetInstance().loadDocTimeStamp) {
        return false;
    }
    ILOG("loadDoc: flushEmptyTimeStamp:%" PRIu64 ", loadDocTimeStamp:%" PRIu64 "",
        timeStamp, GetInstance().loadDocTimeStamp);
    GetInstance().isFlushEmpty = true;
    GetInstance().flushEmptyTime = std::chrono::system_clock::now();
    GetInstance().flushEmptyTimeStamp = timeStamp;
    return true;
}

void VirtualScreenImpl::InitFlushEmptyTime()
{
    GetInstance().isFlushEmpty = false;
    GetInstance().flushEmptyTime = std::chrono::system_clock::time_point::min();
    GetInstance().onRenderTime = std::chrono::system_clock::time_point::min();
    GetInstance().flushEmptyTimeStamp = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    loadDocTimeStamp = ts.tv_sec * SEC_TO_NANOSEC + ts.tv_nsec;
    ILOG("loadDoc: loadDocTimeStamp:%" PRIu64 "", loadDocTimeStamp);
}

bool VirtualScreenImpl::PageCallback(const std::string currentRouterPath)
{
    std::string currentRouter = currentRouterPath.substr(0, currentRouterPath.size() - 3);
    ILOG("PageCallback currentPage is : %s", currentRouter.c_str());
    GetInstance().SetCurrentRouter(currentRouter);
    Json2::Value val;
    CommandLineInterface::GetInstance().CreatCommandToSendData("CurrentRouter", val, "get");
    return true;
}

bool VirtualScreenImpl::LoadContentCallback(const std::string currentRouterPath)
{
    ILOG("LoadContentCallback currentPage is : %s", currentRouterPath.c_str());
    GetInstance().SetAbilityCurrentRouter(currentRouterPath);
    Json2::Value val;
    CommandLineInterface::GetInstance().CreatCommandToSendData("LoadContent", val, "get");
    return true;
}

void VirtualScreenImpl::FastPreviewCallback(const std::string& jsonStr)
{
    GetInstance().SetFastPreviewMsg(jsonStr);
    Json2::Value val;
    CommandLineInterface::GetInstance().CreatCommandToSendData("FastPreviewMsg", val, "get");
}

void VirtualScreenImpl::AvoidAreaChangedCallback(const std::string& jsonStr)
{
    Json2::Value val = JsonReader::ParseJsonData2(jsonStr);
    CommandLineInterface::GetInstance().CreatCommandToSendData("AvoidAreaChanged", val, "get");
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
            int inputBasePos = i * retWidth * pixelSize + j * pixelSize;
            int nowBasePos = i * retWidth * jpgPix + j * jpgPix;
            dataTemp[nowBasePos + redPos] = *((char*)data + inputBasePos + redPos);
            dataTemp[nowBasePos + greenPos] = *((char*)data + inputBasePos + greenPos);
            dataTemp[nowBasePos + bluePos] = *((char*)data + inputBasePos + bluePos);
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
    if (loadDocCopyBuffer != nullptr) {
        delete [] loadDocCopyBuffer;
        loadDocCopyBuffer = nullptr;
    }
    if (loadDocTempBuffer != nullptr) {
        delete [] loadDocTempBuffer;
        loadDocTempBuffer = nullptr;
    }
}

ScreenInfo VirtualScreenImpl::GetScreenInfo()
{
    ScreenInfo info;
    info.orignalResolutionWidth = GetOrignalWidth();
    info.orignalResolutionHeight = GetOrignalHeight();
    info.compressionResolutionWidth = GetCompressionWidth();
    info.compressionResolutionHeight = GetCompressionHeight();
    info.foldStatus = GetFoldStatus();
    info.foldable = GetFoldable();
    info.foldWidth = GetFoldWidth();
    info.foldHeight = GetFoldHeight();
    return info;
}

void VirtualScreenImpl::InitFoldParams()
{
    CommandParser& parser = CommandParser::GetInstance();
    FoldInfo info;
    parser.GetFoldInfo(info);
    if (parser.IsSet("foldable")) {
        SetFoldable(info.foldable);
    }
    if (parser.IsSet("foldStatus")) {
        SetFoldStatus(info.foldStatus);
    }
    if (parser.IsSet("fr")) {
        SetFoldResolution(info.foldResolutionWidth, info.foldResolutionHeight);
    }
}