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

#ifndef VIRTUALSREENIMPL_H
#define VIRTUALSREENIMPL_H

#include "VirtualScreen.h"

class ScreenInfo {
public:
    int32_t orignalResolutionWidth;
    int32_t orignalResolutionHeight;
    int32_t compressionResolutionWidth;
    int32_t compressionResolutionHeight;
    std::string foldStatus;
    bool foldable;
    int32_t foldWidth;
    int32_t foldHeight;
};

class VirtualScreenImpl : public VirtualScreen {
public:
    VirtualScreenImpl(const VirtualScreenImpl&) = delete;
    VirtualScreenImpl& operator=(const VirtualScreenImpl&) = delete;
    static VirtualScreenImpl& GetInstance();
    static void StartTimer();
    static bool FlushEmptyFunc(std::chrono::system_clock::time_point endTime, int64_t timePassed);
    static bool NoFlushEmptyFunc(int64_t timePassed);
    static void PrintLoadDocFinishedLog(const std::string& logStr);
    static void SendBufferOnTimer();
    static bool LoadDocCallback(const void* data, const size_t length,
                                const int32_t width, const int32_t height, const uint64_t timeStamp);
    static bool Callback(const void* data, const size_t length, const int32_t width, const int32_t height,
        const uint64_t timeStamp);
    static bool FlushEmptyCallback(const uint64_t timeStamp);
    void InitFlushEmptyTime() override;
    static bool PageCallback(const std::string currentRouterPath);
    static bool LoadContentCallback(const std::string currentRouterPath);
    static void FastPreviewCallback(const std::string& jsonStr);
    void InitAll(std::string pipeName, std::string pipePort);
    ScreenInfo GetScreenInfo();
    void InitFoldParams();
    void MakeScreenShot(const std::string &fileName);
private:
    VirtualScreenImpl();
    ~VirtualScreenImpl();
    void Send(const void* data, int32_t retWidth, int32_t retHeight);
    void SendRgba(const void* data, size_t length);
    void BackupAndDeleteBuffer(const unsigned long imageBufferSize);
    bool JudgeBeforeSend(const void* data);
    bool SendPixmap(const void* data, size_t length, int32_t retWidth, int32_t retHeight);
    void FreeJpgMemory();
    template<class T, class = typename std::enable_if<std::is_integral<T>::value>::type>
    void WriteBuffer(const T data)
    {
        T dataToSend = EndianUtil::ToNetworkEndian<T>(data);
        unsigned char* startPos = reinterpret_cast<unsigned char*>(&dataToSend);
        std::copy(startPos, startPos + sizeof(dataToSend), screenBuffer + currentPos);
        currentPos += sizeof(dataToSend);
    }
    void UpdateScreenshotBuffer();

    bool isFirstSend;
    bool isFirstRender;
    size_t writed;
    uint8_t* wholeBuffer;
    uint8_t* screenBuffer;
    uint64_t bufferSize;
    unsigned long long currentPos;
    static constexpr int SEND_IMG_DURATION_MS = 300;
    static constexpr int STOP_SEND_CARD_DURATION_MS = 10000;

    uint8_t* loadDocTempBuffer;
    uint8_t* loadDocCopyBuffer;
    size_t lengthTemp;
    int32_t widthTemp;
    int32_t heightTemp;
    uint64_t timeStampTemp;

    static constexpr int TIMEOUT_ONRENDER_DURATION_MS = 100;
    static constexpr int TIMEOUT_NINE_S = 9000;
    static constexpr int64_t SEC_TO_NANOSEC = 1000000000;
    bool isFlushEmpty = false;
    uint64_t loadDocTimeStamp = 0;
    uint64_t flushEmptyTimeStamp = 0;
    std::chrono::system_clock::time_point flushEmptyTime = std::chrono::system_clock::time_point::min();
    std::chrono::system_clock::time_point onRenderTime = std::chrono::system_clock::time_point::min();

    uint8_t *screenShotBuffer = nullptr;
    size_t screenShotBufferSize = 0;
};

#endif // VIRTUALSREENIMPL_H
