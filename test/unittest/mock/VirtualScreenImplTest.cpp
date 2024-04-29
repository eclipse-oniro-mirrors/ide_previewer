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

#include <string>
#include <vector>
#include "gtest/gtest.h"
#define private public
#define protected public
#include "VirtualScreenImpl.h"
#include "MockGlobalResult.h"
#include "CommandParser.h"
#include "VirtualScreen.h"

namespace {
    class VirtualScreenImplTest : public ::testing::Test {
    public:
        VirtualScreenImplTest() {}
        ~VirtualScreenImplTest() {}
    protected:
        static void InitBuffer()
        {
            int retHeight = 100;
            int retWidth = 100;
            int jpgPix = 4;
            int pixelSize = 4;
            uint8_t defaultVal = 200;
            jpgWidth = retWidth;
            jpgHeight = retHeight;
            jpgBuffSize = static_cast<long>(retWidth) * static_cast<long>(retHeight) * static_cast<long>(jpgPix);
            jpgBuff = new unsigned char[jpgBuffSize];
            for (int i = 0; i < jpgBuffSize ; i++) {
                jpgBuff[i] = defaultVal;
            }
        }

        static void SetUpTestCase()
        {
            socket = std::make_unique<LocalSocket>();
        }

        static std::unique_ptr<LocalSocket> socket;
        static unsigned char* jpgBuff;
        static unsigned long jpgBuffSize;
        static int32_t jpgWidth;
        static int32_t jpgHeight;
    };

    std::unique_ptr<LocalSocket> VirtualScreenImplTest::socket = nullptr;
    unsigned char* VirtualScreenImplTest::jpgBuff = nullptr;
    unsigned long VirtualScreenImplTest::jpgBuffSize = 0;
    int32_t VirtualScreenImplTest::jpgWidth = 0;
    int32_t VirtualScreenImplTest::jpgHeight = 0;

    // 测试拷贝构造函数是否被删除
    TEST_F(VirtualScreenImplTest, CopyConstructorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_constructible<VirtualScreenImpl>::value == false);
    }

    // 测试赋值运算符是否被删除
    TEST_F(VirtualScreenImplTest, AssignmentOperatorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_assignable<VirtualScreenImpl>::value == false);
    }

    // VirtualScreen start
    TEST_F(VirtualScreenImplTest, SetOrignalWidthTest)
    {
        int32_t width = 100;
        VirtualScreenImpl::GetInstance().SetOrignalWidth(width);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetOrignalWidth(), width);
    }

    TEST_F(VirtualScreenImplTest, SetCurrentRouterTest)
    {
        std::string router = "aaa";
        VirtualScreenImpl::GetInstance().SetCurrentRouter(router);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetCurrentRouter(), router);
    }

    TEST_F(VirtualScreenImplTest, SetAbilityCurrentRouterTest)
    {
        std::string router = "aaa";
        VirtualScreenImpl::GetInstance().SetAbilityCurrentRouter(router);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetAbilityCurrentRouter(), router);
    }

    TEST_F(VirtualScreenImplTest, SetOrignalHeightTest)
    {
        int32_t width = 100;
        VirtualScreenImpl::GetInstance().SetOrignalHeight(width);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetOrignalHeight(), width);
    }

    TEST_F(VirtualScreenImplTest, SetCompressionWidthTest)
    {
        int32_t width = 100;
        VirtualScreenImpl::GetInstance().SetCompressionWidth(width);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetCompressionWidth(), width);
    }

    TEST_F(VirtualScreenImplTest, SetCompressionHeightTest)
    {
        int32_t width = 100;
        VirtualScreenImpl::GetInstance().SetCompressionHeight(width);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetCompressionHeight(), width);
    }

    TEST_F(VirtualScreenImplTest, InitPipeTest)
    {
        std::string port = "8888";
        g_run = false;
        VirtualScreenImpl::GetInstance().InitPipe("aaa", port);
        EXPECT_EQ(WebSocketServer::GetInstance().serverPort, atoi(port.c_str()));
        EXPECT_TRUE(g_run);
    }

    TEST_F(VirtualScreenImplTest, InitVirtualScreenTest)
    {
        int width = 1111;
        int height = 2222;
        CommandParser::GetInstance().orignalResolutionWidth = width;
        CommandParser::GetInstance().orignalResolutionHeight = height;
        CommandParser::GetInstance().compressionResolutionWidth = width;
        CommandParser::GetInstance().compressionResolutionHeight = height;
        VirtualScreenImpl::GetInstance().InitVirtualScreen();
        EXPECT_EQ(VirtualScreenImpl::GetInstance().orignalResolutionWidth, width);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().orignalResolutionHeight, height);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().compressionResolutionWidth, width);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().compressionResolutionHeight, height);
    }

    TEST_F(VirtualScreenImplTest, InitFrameCountTimerTest)
    {
        VirtualScreenImpl::GetInstance().inputMethodCountPerMinute = 0;
        VirtualScreenImpl::GetInstance().InitFrameCountTimer();
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().frameCountTimer == nullptr);

        VirtualScreenImpl::GetInstance().InitFrameCountTimer();
        EXPECT_EQ(VirtualScreenImpl::GetInstance().inputMethodCountPerMinute, 0);
    }

    TEST_F(VirtualScreenImplTest, PrintFrameCountTest)
    {
        VirtualScreenImpl::GetInstance().inputMethodCountPerMinute = 0;
        VirtualScreenImpl::GetInstance().PrintFrameCount();
        EXPECT_EQ(VirtualScreenImpl::GetInstance().inputMethodCountPerMinute, 0);

        VirtualScreenImpl::GetInstance().inputMethodCountPerMinute = 1;
        VirtualScreenImpl::GetInstance().PrintFrameCount();
        EXPECT_EQ(VirtualScreenImpl::GetInstance().inputMethodCountPerMinute, 0);
    }

    TEST_F(VirtualScreenImplTest, WidthAndHeightReverseTest)
    {
        int32_t width = 1080;
        int32_t height = 2340;
        VirtualScreenImpl::GetInstance().orignalResolutionHeight = height;
        VirtualScreenImpl::GetInstance().orignalResolutionWidth = width;
        VirtualScreenImpl::GetInstance().compressionResolutionHeight = height;
        VirtualScreenImpl::GetInstance().compressionResolutionWidth = width;
        VirtualScreenImpl::GetInstance().WidthAndHeightReverse();
        EXPECT_EQ(VirtualScreenImpl::GetInstance().orignalResolutionHeight, width);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().orignalResolutionWidth, height);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().compressionResolutionHeight, width);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().compressionResolutionWidth, height);
    }

    TEST_F(VirtualScreenImplTest, SetVirtualScreenWidthAndHeightTest)
    {
        int32_t width = 1080;
        int32_t height = 2340;
        VirtualScreenImpl::GetInstance().orignalResolutionHeight = height;
        VirtualScreenImpl::GetInstance().orignalResolutionWidth = width;
        VirtualScreenImpl::GetInstance().compressionResolutionHeight = height;
        VirtualScreenImpl::GetInstance().compressionResolutionWidth = width;
        int32_t newWidth = 2000;
        int32_t newHeight = 3000;
        VirtualScreenImpl::GetInstance().SetVirtualScreenWidthAndHeight(newWidth, newHeight, newWidth, newHeight);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().orignalResolutionHeight, newHeight);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().orignalResolutionWidth, newWidth);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().compressionResolutionHeight, newHeight);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().compressionResolutionWidth, newWidth);
    }

    TEST_F(VirtualScreenImplTest, GetJpgQualityValueTest)
    {
        // <= 100000
        int32_t width = 200;
        int32_t height = 300;
        int ret = VirtualScreenImpl::GetInstance().GetJpgQualityValue(width, height);
        EXPECT_EQ(ret, 100); // 100 is jpeg quality
        // <= 300000 && > 100000
        width = 400;
        height = 500;
        ret = VirtualScreenImpl::GetInstance().GetJpgQualityValue(width, height);
        EXPECT_EQ(ret, 90); // 90 is jpeg quality
        width = 500;
        height = 600;
        ret = VirtualScreenImpl::GetInstance().GetJpgQualityValue(width, height);
        EXPECT_EQ(ret, 90); // 90 is jpeg quality
        // <= 500000 && > 300000
        width = 600;
        height = 700;
        ret = VirtualScreenImpl::GetInstance().GetJpgQualityValue(width, height);
        EXPECT_EQ(ret, 85); // 85 is jpeg quality
        width = 500;
        height = 1000;
        ret = VirtualScreenImpl::GetInstance().GetJpgQualityValue(width, height);
        EXPECT_EQ(ret, 85); // 85 is jpeg quality
        // > 500000
        width = 700;
        height = 800;
        ret = VirtualScreenImpl::GetInstance().GetJpgQualityValue(width, height);
        EXPECT_EQ(ret, 75); // 75 is jpeg quality
    }

    TEST_F(VirtualScreenImplTest, SetLoadDocFlagTest)
    {
        VirtualScreenImpl::GetInstance().startLoadDoc = VirtualScreen::LoadDocType::INIT;
        VirtualScreenImpl::GetInstance().SetLoadDocFlag(VirtualScreen::LoadDocType::FINISHED);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetLoadDocFlag(), VirtualScreen::LoadDocType::FINISHED);
    }

    TEST_F(VirtualScreenImplTest, GetFastPreviewMsgTest)
    {
        VirtualScreenImpl::GetInstance().fastPreviewMsg = "";
        VirtualScreenImpl::GetInstance().SetFastPreviewMsg("FastPreviewMsg");
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetFastPreviewMsg(), "FastPreviewMsg");
    }

    TEST_F(VirtualScreenImplTest, JudgeAndDropFrameTest)
    {
        int frequency = 0; // 0 ms
        VirtualScreenImpl::GetInstance().SetDropFrameFrequency(frequency);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().dropFrameFrequency, frequency);
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().JudgeAndDropFrame());

        auto time1 = std::chrono::system_clock::now();
        frequency = 1; // 1 ms
        VirtualScreenImpl::GetInstance().SetDropFrameFrequency(frequency);
        while (VirtualScreenImpl::GetInstance().JudgeAndDropFrame()) {
            EXPECT_TRUE(VirtualScreenImpl::GetInstance().startDropFrameTime > time1);
        }
    }

    TEST_F(VirtualScreenImplTest, JudgeStaticImageTest)
    {
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::DYNAMIC;
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().JudgeStaticImage(1));

        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::STATIC;
        VirtualScreen::isOutOfSeconds = true;
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().JudgeStaticImage(1));

        VirtualScreen::isOutOfSeconds = false;
        VirtualScreen::isStartCount = true;
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().JudgeStaticImage(0));
    }

    TEST_F(VirtualScreenImplTest, StopSendStaticCardImageTest)
    {
        CommandParser::GetInstance().staticCard = false;
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().StopSendStaticCardImage(0));

        CommandParser::GetInstance().staticCard = true;
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().StopSendStaticCardImage(-1));
    }

    TEST_F(VirtualScreenImplTest, RgbToJpgTest)
    {
        InitBuffer();
        VirtualScreenImpl::GetInstance().jpgBufferSize = 0;
        VirtualScreenImpl::GetInstance().RgbToJpg(jpgBuff, 3, 3);
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().jpgBufferSize > 0);
        delete[] jpgBuff;
        jpgBuff = nullptr;
        if (VirtualScreenImpl::GetInstance().jpgScreenBuffer) {
            free(VirtualScreenImpl::GetInstance().jpgScreenBuffer);
            VirtualScreenImpl::GetInstance().jpgScreenBuffer = NULL;
            VirtualScreenImpl::GetInstance().jpgBufferSize = 0;
        }
    }

    TEST_F(VirtualScreenImplTest, SetFoldableTest)
    {
        VirtualScreenImpl::GetInstance().SetFoldable(true);
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().GetFoldable());

        VirtualScreenImpl::GetInstance().SetFoldable(false);
        EXPECT_FALSE(VirtualScreenImpl::GetInstance().GetFoldable());
    }

    TEST_F(VirtualScreenImplTest, SetFoldStatusTest)
    {
        VirtualScreenImpl::GetInstance().SetFoldStatus("fold");
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetFoldStatus(), "fold");

        VirtualScreenImpl::GetInstance().SetFoldStatus("unfold");
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetFoldStatus(), "unfold");
    }

    TEST_F(VirtualScreenImplTest, SetFoldResolutionTest)
    {
        int32_t foldWidth = 300;
        int32_t foldHeight = 400;
        VirtualScreenImpl::GetInstance().SetFoldResolution(foldWidth, foldHeight);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetFoldWidth(), foldWidth);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetFoldHeight(), foldHeight);
    }

    TEST_F(VirtualScreenImplTest, SetCurrentResolutionTest)
    {
        int32_t foldWidth = 300;
        int32_t foldHeight = 400;
        VirtualScreenImpl::GetInstance().SetCurrentResolution(foldWidth, foldHeight);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetCurrentWidth(), foldWidth);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().GetCurrentHeight(), foldHeight);
    }
    // VirtualScreen end

    TEST_F(VirtualScreenImplTest, InitFlushEmptyTimeTest)
    {
        VirtualScreenImpl::GetInstance().loadDocTimeStamp = 0;
        VirtualScreenImpl::GetInstance().InitFlushEmptyTime();
        EXPECT_TRUE(VirtualScreenImpl::GetInstance().loadDocTimeStamp > 0);
    }

    TEST_F(VirtualScreenImplTest, SendBufferOnTimerTest)
    {
        g_writeData = false;
        VirtualScreenImpl::GetInstance().loadDocTempBuffer = nullptr;
        VirtualScreenImpl::GetInstance().SendBufferOnTimer();

        InitBuffer();
        VirtualScreenImpl::GetInstance().lengthTemp = jpgBuffSize;
        VirtualScreenImpl::GetInstance().loadDocTempBuffer = jpgBuff;
        VirtualScreenImpl::GetInstance().widthTemp = jpgWidth;
        VirtualScreenImpl::GetInstance().heightTemp = jpgHeight;
        VirtualScreenImpl::GetInstance().loadDocCopyBuffer = new unsigned char[0];
        VirtualScreenImpl::GetInstance().SendBufferOnTimer();
        EXPECT_TRUE(g_writeData);
    }

    TEST_F(VirtualScreenImplTest, CallbackTest)
    {
        CommandParser::GetInstance().staticCard = false;
        VirtualScreenImpl::GetInstance().loadDocTimeStamp = 0;
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::DYNAMIC;
        VirtualScreenImpl::GetInstance().SetLoadDocFlag(VirtualScreen::LoadDocType::INIT);
        g_writeData = false;
        InitBuffer();
        int tm = 100;
        VirtualScreenImpl::Callback(jpgBuff, jpgBuffSize, jpgWidth, jpgHeight, tm);
        EXPECT_TRUE(g_writeData);
    }

    TEST_F(VirtualScreenImplTest, FlushEmptyCallbackTest)
    {
        int tm = 100;
        VirtualScreenImpl::GetInstance().loadDocTimeStamp = tm;
        tm = 10;
        VirtualScreenImpl::GetInstance().FlushEmptyCallback(tm);
        EXPECT_NE(VirtualScreenImpl::GetInstance().flushEmptyTimeStamp, tm);
        tm = 200;
        VirtualScreenImpl::GetInstance().FlushEmptyCallback(tm);
        EXPECT_EQ(VirtualScreenImpl::GetInstance().flushEmptyTimeStamp, tm);
    }

    TEST_F(VirtualScreenImplTest, InitAllTest)
    {
        std::string port = "8888";
        VirtualScreenImpl::GetInstance().InitAll("aaa", port);
        EXPECT_EQ(WebSocketServer::GetInstance().serverPort, atoi(port.c_str()));
    }
}