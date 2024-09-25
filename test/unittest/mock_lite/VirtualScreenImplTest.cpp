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

#include "gtest/gtest.h"
#define private public
#define protected public
#include "VirtualScreenImpl.h"
#include "CommandParser.h"

namespace {
    class VirtualScreenImplTest : public ::testing::Test {
    public:
        VirtualScreenImplTest() {}
        ~VirtualScreenImplTest() {}
    protected:
        static void InitBuffer()
        {
            int retWidth = 100;
            int retHeight = 100;
            int jpgPix = 4;
            int pixelSize = 4;
            uint8_t defaultValue = 200;
            jpgHeight = retHeight;
            jpgWidth = retWidth;
            jpgBufferSize = static_cast<long>(retWidth) * static_cast<long>(retHeight) * static_cast<long>(jpgPix);
            jpgBuffer = new unsigned char[jpgBufferSize];
            for (int i = 0; i < jpgBufferSize; i++) {
                jpgBuffer[i] = defaultValue;
            }
        }

        static void SetUpTestCase()
        {
            InitBuffer();
            socket = std::make_unique<LocalSocket>();
        }

        static std::unique_ptr<LocalSocket> socket;
        static unsigned long jpgBufferSize;
        static unsigned char* jpgBuffer;
        static int32_t jpgWidth;
        static int32_t jpgHeight;
    };

    std::unique_ptr<LocalSocket> VirtualScreenImplTest::socket = nullptr;
    unsigned char* VirtualScreenImplTest::jpgBuffer = nullptr;
    unsigned long VirtualScreenImplTest::jpgBufferSize = 0;
    int32_t VirtualScreenImplTest::jpgWidth = 0;
    int32_t VirtualScreenImplTest::jpgHeight = 0;

    TEST_F(VirtualScreenImplTest, IsRectValidTest)
    {
        int32_t width = 100;
        int32_t height = 200;
        VirtualScreenImpl::GetInstance().SetOrignalWidth(width);
        VirtualScreenImpl::GetInstance().SetOrignalHeight(height);
        int32_t x = -1;
        int32_t y = 100;
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().IsRectValid(x, y, width, height));

        x = 50;
        y = -1;
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().IsRectValid(x, y, width, height));

        x = 50;
        y = 100;
        width = 100;
        height = 100;
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().IsRectValid(x, y, width, height));

        width = 50;
        height = 200;
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().IsRectValid(x, y, width, height));

        width = 50;
        height = 100;
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().IsRectValid(x, y, width, height));
    }

    TEST_F(VirtualScreenImplTest, GetScreenWidthTest)
    {
        int32_t width = 100;
        VirtualScreenImpl::GetInstance().SetOrignalWidth(width);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetScreenWidth(),
            VirtualScreenImpl::GetInstance().GetOrignalWidth());
    }

    TEST_F(VirtualScreenImplTest, GetScreenHeightTest)
    {
        int32_t height = 200;
        VirtualScreenImpl::GetInstance().SetOrignalHeight(height);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetScreenHeight(),
            VirtualScreenImpl::GetInstance().GetOrignalHeight());
    }

    TEST_F(VirtualScreenImplTest, InitAllTest)
    {
        CommandParser::GetInstance().deviceType = "liteWearable";
        int32_t height = -1;
        VirtualScreenImpl::GetInstance().SetOrignalWidth(jpgWidth);
        VirtualScreenImpl::GetInstance().SetOrignalHeight(height);
        VirtualScreenImpl::GetInstance().InitAll("aaa", "8888");
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().wholeBuffer == nullptr);
        VirtualScreenImpl::GetInstance().SetOrignalHeight(jpgHeight);
        VirtualScreenImpl::GetInstance().InitAll("aaa", "8888");
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().wholeBuffer != nullptr);
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().currentPos > 0);
    }

    TEST_F(VirtualScreenImplTest, GetFBBufferInfoTest)
    {
        OHOS::BufferInfo* info = VirtualScreenImpl::GetInstance().GetFBBufferInfo();
        EXPECT_EQ(info->color, 0x44);
    }

    TEST_F(VirtualScreenImplTest, UpdateRegionTest)
    {
        int32_t width = 100;
        int32_t height = 200;
        VirtualScreenImpl::GetInstance().SetCompressionWidth(width);
        VirtualScreenImpl::GetInstance().SetCompressionHeight(height);
        int32_t x1 = 10;
        int32_t y1 = 10;
        int32_t x2 = 20;
        int32_t y2 = 20;
        VirtualScreenImpl::GetInstance().UpdateRegion(x1, y1, x2, y2);
        int32_t expectVal = 26;
        EXPECT_EQ(VirtualScreenImpl::GetInstance().regionWidth, expectVal);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().regionHeight, expectVal);
    }

    TEST_F(VirtualScreenImplTest, SendRegionBufferTest)
    {
        VirtualScreenImpl::GetInstance().SetCompressionWidth(jpgWidth);
        VirtualScreenImpl::GetInstance().SetCompressionHeight(jpgHeight);

        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::STATIC;
        VirtualScreen::isOutOfSeconds = true;
        VirtualScreenImpl::GetInstance().SendRegionBuffer();
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().jpgScreenBuffer == nullptr);
        VirtualScreenImpl::GetInstance().FreeJpgMemory();
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::DYNAMIC;
        VirtualScreen::isOutOfSeconds = false;
        VirtualScreenImpl::GetInstance().SendRegionBuffer();
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().jpgScreenBuffer == nullptr);
    }

    TEST_F(VirtualScreenImplTest, FlushTest)
    {
        VirtualScreenImpl::GetInstance().SetCompressionWidth(jpgWidth);
        VirtualScreenImpl::GetInstance().SetCompressionHeight(jpgHeight);
        OHOS::Rect flushRect;

        VirtualScreenImpl::GetInstance().isWebSocketConfiged = false;
        VirtualScreenImpl::GetInstance().Flush(flushRect);
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().isChanged);

        VirtualScreenImpl::GetInstance().isWebSocketConfiged = true;
        CommandParser::GetInstance().isRegionRefresh = true;
        VirtualScreenImpl::GetInstance().isFirstSend = false;
        VirtualScreenImpl::GetInstance().Flush(flushRect);
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().isChanged);

        CommandParser::GetInstance().isRegionRefresh = false;
        VirtualScreenImpl::GetInstance().isFirstSend = false;
        VirtualScreenImpl::GetInstance().Flush(flushRect);
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().isChanged);
    }

    TEST_F(VirtualScreenImplTest, CheckBufferSendTest)
    {
        VirtualScreenImpl::GetInstance().SetCompressionWidth(jpgWidth);
        VirtualScreenImpl::GetInstance().SetCompressionHeight(jpgHeight);

        VirtualScreenImpl::GetInstance().isChanged = false;
        VirtualScreenImpl::GetInstance().CheckBufferSend();
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().isChanged);

        VirtualScreenImpl::GetInstance().isChanged = true;
        VirtualScreenImpl::GetInstance().isWebSocketConfiged = false;
        VirtualScreenImpl::GetInstance().CheckBufferSend();
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().isChanged);

        VirtualScreenImpl::GetInstance().isWebSocketConfiged = true;
        CommandParser::GetInstance().isRegionRefresh = true;
        VirtualScreenImpl::GetInstance().isFirstSend = false;
        VirtualScreenImpl::GetInstance().CheckBufferSend();
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().isChanged);

        CommandParser::GetInstance().isRegionRefresh = false;
        VirtualScreenImpl::GetInstance().isFirstSend = false;
        VirtualScreenImpl::GetInstance().CheckBufferSend();
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().isChanged);
    }
}