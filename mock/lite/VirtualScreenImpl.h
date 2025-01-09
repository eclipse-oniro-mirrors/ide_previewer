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

#include "engines/gfx/soft_engine.h"
#include "gfx_utils/color.h"
#include "input_device.h"

#include "EndianUtil.h"
#include "LocalSocket.h"
#include "VirtualScreen.h"

class VirtualScreenImpl : public OHOS::SoftEngine, public VirtualScreen {
public:
    enum class MouseStatus { INDEV_STATE_RELEASE = 0, INDEV_STATE_PRESS };

    static VirtualScreenImpl& GetInstance();
    static void CheckBufferSend();
    VirtualScreenImpl(const VirtualScreenImpl&) = delete;
    VirtualScreenImpl& operator=(const VirtualScreenImpl&) = delete;
    void Flush(const OHOS::Rect& flushRect) override;
    OHOS::BufferInfo* GetFBBufferInfo() override;
    uint16_t GetScreenWidth() override;
    uint16_t GetScreenHeight() override;
    void InitBuffer();
    void InitAll(std::string pipeName, std::string pipePort);
    void MakeScreenShot(const std::string &fileName) {}
private:
    VirtualScreenImpl();
    ~VirtualScreenImpl();
    bool IsRectValid(int32_t x1, int32_t y1, int32_t x2, int32_t y2) const;
    uint8_t* wholeBuffer;
    uint8_t* regionWholeBuffer;
    uint8_t* screenBuffer;
    uint8_t* regionBuffer;
    uint8_t* osBuffer;
    bool isChanged;
    void ScheduleBufferSend();
    void Send(unsigned char* data, int32_t width, int32_t height);
    void SendFullBuffer();
    void SendRegionBuffer();
    void FreeJpgMemory();

    template <class T, class = typename std::enable_if<std::is_integral<T>::value>::type>
    void WriteBuffer(const T data)
    {
        T dataToSend = EndianUtil::ToNetworkEndian<T>(data);
        char* startPos = reinterpret_cast<char*>(&dataToSend);
        std::copy(startPos, startPos + sizeof(dataToSend), screenBuffer + currentPos);
        currentPos += sizeof(dataToSend);
    }

    void WriteRefreshRegion();
    void UpdateRegion(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

    unsigned long long currentPos;
    uint64_t bufferSize;
    bool isFirstRender;
    bool isFirstSend;
    const uint16_t VERSION_POS = 20;
    int16_t regionX1;
    int16_t regionY1;
    int16_t regionX2;
    int16_t regionY2;
    int16_t regionWidth;
    int16_t regionHeight;
    int32_t extendPix = 15;
    OHOS::BufferInfo* bufferInfo;
    static constexpr int SEND_IMG_DURATION_MS = 300;
};

#endif // VIRTUALSREENIMPL_H
