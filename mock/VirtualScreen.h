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

#ifndef VIRTUALSCREEN_H
#define VIRTUALSCREEN_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

#include "CppTimer.h"
#include "LocalSocket.h"
#include "WebSocketServer.h"

class VirtualScreen {
public:
    VirtualScreen();
    virtual ~VirtualScreen();

    int32_t GetOrignalWidth() const;
    void SetOrignalWidth(const int32_t& value);

    std::string GetCurrentRouter() const;
    void SetCurrentRouter(const std::string currentRouterValue);

    std::string GetAbilityCurrentRouter() const;
    void SetAbilityCurrentRouter(const std::string currentRouterValue);

    int32_t GetOrignalHeight() const;
    void SetOrignalHeight(const int32_t& value);

    int32_t GetCompressionWidth() const;
    void SetCompressionWidth(const int32_t& value);

    int32_t GetCompressionHeight() const;
    void SetCompressionHeight(const int32_t& value);

    void InitPipe(std::string pipeName, std::string pipePort);

    void InitVirtualScreen();

    void InitFrameCountTimer();

    static void PrintFrameCount();

    std::atomic<bool> isFrameUpdated;
    static bool isWebSocketListening;
    static std::string webSocketPort;

    void WidthAndHeightReverse();

    void SetVirtualScreenWidthAndHeight(const int32_t& orignalWidth,
                                        const int32_t& orignalHeight,
                                        const int32_t& compressionWidth,
                                        const int32_t& compressionHeight);

    int GetJpgQualityValue(int32_t width, int32_t height) const;

    enum class LoadDocType { INIT = 3, START = 1, FINISHED = 2, NORMAL = 0 };
    void SetLoadDocFlag(VirtualScreen::LoadDocType flag);
    VirtualScreen::LoadDocType GetLoadDocFlag() const;

    enum class ProtocolVersion { LOADNORMAL = 2, LOADDOC = 3 };

    enum class JpgPixCountLevel { LOWCOUNT = 100000, MIDDLECOUNT = 300000, HIGHCOUNT = 500000};
    enum class JpgQualityLevel { HIGHLEVEL = 100, MIDDLELEVEL = 90, LOWLEVEL = 85, DEFAULTLEVEL = 75};

    static bool isOutOfSeconds;
    static bool isStartCount;

    std::string GetFastPreviewMsg() const;
    void SetFastPreviewMsg(const std::string msg);
    bool JudgeAndDropFrame();
    void SetDropFrameFrequency(const int32_t& value);
    static bool JudgeStaticImage(const int duration);
    static bool StopSendStaticCardImage(const int duration);
    void RgbToJpg(unsigned char* data, const int32_t width, const int32_t height);
    static uint32_t inputKeyCountPerMinute;
    static uint32_t inputMethodCountPerMinute;

    void SetFoldable(const bool value);
    bool GetFoldable() const;
    void SetFoldStatus(const std::string& value);
    std::string GetFoldStatus() const;
    void SetFoldResolution(int32_t changedFoldWidth, int32_t changedFoldHeight);
    int32_t GetFoldWidth() const;
    int32_t GetFoldHeight() const;
    void SetCurrentResolution(int32_t width, int32_t height);
    int32_t GetCurrentWidth() const;
    int32_t GetCurrentHeight() const;
    virtual void InitFlushEmptyTime();

protected:
    // start width and height
    int32_t orignalResolutionWidth;
    int32_t orignalResolutionHeight;
    int32_t compressionResolutionWidth;
    int32_t compressionResolutionHeight;
    int32_t foldWidth = 0;
    int32_t foldHeight = 0;
    // width and height after resize
    int32_t currentWidth = 0;
    int32_t currentHeight = 0;
    std::string foldStatus = "unfold";
    bool foldable = false;
    static uint32_t validFrameCountPerMinute;
    static uint32_t invalidFrameCountPerMinute;
    static uint32_t sendFrameCountPerMinute;

    LocalSocket* screenSocket;
    std::unique_ptr<CppTimer> frameCountTimer;

    const int32_t sendPeriod = 40;              // A frame is sent per 40 ms.
    const uint16_t pixelSize = 4;               // 4 bytes per pixel
    const size_t headSize = 40;                 // The packet header length is 40 bytes.
    const size_t headReservedSize = 20;         // The reserved length of the packet header is 20 bytes.
    const uint32_t headStart = 0x12345678;      // Buffer header starts with magic value 0x12345678
    const int32_t frameCountPeriod = 60 * 1000; // Frame count per minute
    uint16_t protocolVersion = static_cast<uint16_t>(VirtualScreen::ProtocolVersion::LOADNORMAL);
    bool isWebSocketConfiged;
    std::string currentRouter;
    std::string abilityCurrentRouter;
    std::string fastPreviewMsg;
    uint8_t* jpgScreenBuffer;
    unsigned long jpgBufferSize;
    int jpgPix = 3; // jpg color components
    int redPos = 0;
    int greenPos = 1;
    int bluePos = 2;

    static std::chrono::system_clock::time_point startTime;
    static std::chrono::system_clock::time_point staticCardStartTime;
    VirtualScreen::LoadDocType startLoadDoc = VirtualScreen::LoadDocType::INIT;
    std::chrono::system_clock::time_point startDropFrameTime;   // record start drop frame time
    int dropFrameFrequency = 0; // save drop frame frequency
};

#endif // VIRTUALSCREEN_H
