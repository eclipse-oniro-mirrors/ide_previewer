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
#include <map>
#include "gtest/gtest.h"
#define private public
#define protected public
#include "CommandLineFactory.h"
#include "CommandParser.h"
#include "JsAppImpl.h"
#include "MockGlobalResult.h"
#include "VirtualScreenImpl.h"
#include "KeyInputImpl.h"
#include "MouseInputImpl.h"
#include "SharedData.h"
#include "MouseWheelImpl.h"
#include "Interrupter.h"
using namespace std;

namespace {
    class CommandLineTest : public ::testing::Test {
    public:
        CommandLineTest() {}
        ~CommandLineTest() {}
        static std::unique_ptr<LocalSocket> socket;
    protected:
        static void SetUpTestCase()
        {
            socket = std::make_unique<LocalSocket>();
            SharedData<bool>(SharedDataType::KEEP_SCREEN_ON, true);
            SharedData<uint8_t>(SharedDataType::BATTERY_STATUS, (uint8_t)ChargeState::NOCHARGE,
                                (uint8_t)ChargeState::NOCHARGE, (uint8_t)ChargeState::CHARGING);
            // The brightness ranges from 1 to 255. The default value is 255.
            SharedData<uint8_t>(SharedDataType::BRIGHTNESS_VALUE, 255, 1, 255);
            SharedData<uint8_t>(SharedDataType::BRIGHTNESS_MODE, (uint8_t)BrightnessMode::MANUAL,
                                (uint8_t)BrightnessMode::MANUAL, (uint8_t)BrightnessMode::AUTO);
            // The value ranges from 0 to 999999. The default value is 0.
            SharedData<uint32_t>(SharedDataType::SUMSTEP_VALUE, 0, 0, 999999);
            // The volume ranges from 0.0 to 1.0. The default value is 1.0.
            SharedData<double>(SharedDataType::VOLUME_VALUE, 1.0, 0.0, 1.0);
            // Battery level range: 0.0–1.0; default: 1.0
            SharedData<double>(SharedDataType::BATTERY_LEVEL, 1.0, 0.0, 1.0);
            // Heart rate range: 0 to 255. The default value is 80.
            SharedData<uint8_t>(SharedDataType::HEARTBEAT_VALUE, 80, 0, 255);
            SharedData<string>(SharedDataType::LANGUAGE, "zh-CN");
            // The value ranges from 180 to 180. The default value is 0.
            SharedData<double>(SharedDataType::LONGITUDE, 0, -180, 180);
            // The atmospheric pressure ranges from 0 to 999900. The default value is 101325.
            SharedData<uint32_t>(SharedDataType::PRESSURE_VALUE, 101325, 0, 999900);
            SharedData<bool>(SharedDataType::WEARING_STATE, true);
            // The value ranges from -90 to 90. The default value is 0.
            SharedData<double>(SharedDataType::LATITUDE, 0, -90, 90);
        }
    };

    std::unique_ptr<LocalSocket> CommandLineTest::socket = nullptr;


    TEST_F(CommandLineTest, BackClickedCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg = "{\"args\":null}";
        Json2::Value args = JsonReader::ParseJsonData2(msg);
        BackClickedCommand command(type, args, *socket);
        g_dispatchOsBackEvent = false;
        command.CheckAndRun();
        EXPECT_TRUE(g_dispatchOsBackEvent);
    }

    TEST_F(CommandLineTest, InspectorJSONTreeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        Json2::Value args;
        InspectorJSONTree command(type, args, *socket);
        g_getJSONTree = false;
        command.CheckAndRun();
        EXPECT_TRUE(g_getJSONTree);
    }

    TEST_F(CommandLineTest, InspectorDefaultTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        Json2::Value args;
        InspectorDefault command(type, args, *socket);
        g_getDefaultJSONTree = false;
        command.CheckAndRun();
        EXPECT_TRUE(g_getDefaultJSONTree);
    }

    TEST_F(CommandLineTest, OrientationCommandTest)
    {
        JsAppImpl::GetInstance().orientation = "";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        // args是null
        Json2::Value args1 = JsonReader::CreateNull();
        OrientationCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().GetOrientation(), "");
        // 无Orientation
        std::string jsonStr = "{\"aaaa\":\"landscape\"}";
        Json2::Value args2 = JsonReader::ParseJsonData2(jsonStr);
        OrientationCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().GetOrientation(), "");
        // 有Orientation，但不是string类型
        jsonStr = "{\"Orientation\":\"aaaaa\"}";
        Json2::Value args3 = JsonReader::ParseJsonData2(jsonStr);
        OrientationCommand command3(type, args3, *socket);
        command3.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().GetOrientation(), "");
        // Orientation : landscape
        jsonStr = "{\"Orientation\":\"landscape\"}";
        Json2::Value args4 = JsonReader::ParseJsonData2(jsonStr);
        OrientationCommand command4(type, args4, *socket);
        g_output = false;
        command4.CheckAndRun();
        EXPECT_TRUE(g_output);
        EXPECT_EQ(JsAppImpl::GetInstance().GetOrientation(), "landscape");
        // Orientation : portrait
        args4.Replace("Orientation", "portrait");
        OrientationCommand command5(type, args4, *socket);
        command5.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().GetOrientation(), "portrait");
    }

    // 参数异常
    TEST_F(CommandLineTest, ResolutionSwitchCommandArgsTest)
    {
        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        // null
        Json2::Value args1 = JsonReader::CreateNull();
        ResolutionSwitchCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().width, 0);
        EXPECT_EQ(JsAppImpl::GetInstance().height, 0);
        // 缺失参数
        string jsonStr = R"({"aaaaa":1080,"originHeight":2340,"width":1080,"height":2340,"screenDensity":480})";
        Json2::Value args2 = JsonReader::ParseJsonData2(jsonStr);
        ResolutionSwitchCommand command3(type, args2, *socket);
        command3.CheckAndRun();
        EXPECT_NE(JsAppImpl::GetInstance().width, 1080);
        EXPECT_NE(JsAppImpl::GetInstance().height, 2340);
    }

    // 参数类型异常
    TEST_F(CommandLineTest, ResolutionSwitchCommandArgsTypeTest)
    {
        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        string jsonStr = R"({"originWidth":"1080","originHeight":2340,"width":1080,
            "height":2340,"screenDensity":480})";
        Json2::Value args2 = JsonReader::ParseJsonData2(jsonStr);
        ResolutionSwitchCommand command3(type, args2, *socket);
        command3.CheckAndRun();
        EXPECT_NE(JsAppImpl::GetInstance().width, 1080);
        EXPECT_NE(JsAppImpl::GetInstance().height, 2340);

        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        string jsonStr1 = R"({"originWidth" : 1080, "originHeight" : 2340, "width" : 1080,
            "height" : 2340, "screenDensity" : 480, "reason" : 333})";
        Json2::Value args1 = JsonReader::ParseJsonData2(jsonStr1);
        ResolutionSwitchCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().width, 0);
        EXPECT_EQ(JsAppImpl::GetInstance().height, 0);
    }

    // 参数范围异常
    TEST_F(CommandLineTest, ResolutionSwitchCommandArgsRangesTest)
    {
        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        string jsonStr = R"({"originWidth":5000,"originHeight":2340,"width":1080,
            "height":2340,"screenDensity":480})";
        Json2::Value args2 = JsonReader::ParseJsonData2(jsonStr);
        ResolutionSwitchCommand command3(type, args2, *socket);
        command3.CheckAndRun();
        EXPECT_NE(JsAppImpl::GetInstance().width, 1080);
        EXPECT_NE(JsAppImpl::GetInstance().height, 2340);

        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        string jsonStr1 = R"({"originWidth" : 1080, "originHeight" : 2340, "width" : 1080,
            "height" : 2340, "screenDensity" : 480, "reason" : "aaa"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(jsonStr1);
        ResolutionSwitchCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().width, 0);
        EXPECT_EQ(JsAppImpl::GetInstance().height, 0);

        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        string jsonStr4 = R"({"originWidth" : 1080, "originHeight" : 2340, "width" : 1080,
            "height" : 2340, "screenDensity" : 100, "reason" : "resize"})";
        Json2::Value args4 = JsonReader::ParseJsonData2(jsonStr4);
        ResolutionSwitchCommand command4(type, args4, *socket);
        command4.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().width, 0);
        EXPECT_EQ(JsAppImpl::GetInstance().height, 0);

        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        string jsonStr5 = R"({"originWidth" : 1080, "originHeight" : 2340, "width" : 1080,
            "height" : 2340, "screenDensity" : 700, "reason" : "resize"})";
        Json2::Value args5 = JsonReader::ParseJsonData2(jsonStr5);
        ResolutionSwitchCommand command5(type, args5, *socket);
        command5.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().width, 0);
        EXPECT_EQ(JsAppImpl::GetInstance().height, 0);
    }

    // 参数正常
    TEST_F(CommandLineTest, ResolutionSwitchCommandArgsCorrectTest)
    {
        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        string jsonStr = R"({"originWidth" : 1080, "originHeight" : 2340, "width" : 1080,
            "height" : 2340, "screenDensity" : 480, "reason" : "resize"})";
        Json2::Value args2 = JsonReader::ParseJsonData2(jsonStr);
        ResolutionSwitchCommand command3(type, args2, *socket);
        command3.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().width, 1080);
        EXPECT_EQ(JsAppImpl::GetInstance().height, 2340);
    }

    TEST_F(CommandLineTest, CurrentRouterCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args1 = JsonReader::CreateObject();
        CurrentRouterCommand command3(type, args1, *socket);
        g_getCurrentRouter = false;
        command3.CheckAndRun();
        EXPECT_TRUE(g_getCurrentRouter);
    }

    TEST_F(CommandLineTest, ReloadRuntimePageCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg = "{\"ReloadRuntimePage\":\"aaa\"}";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg);
        ReloadRuntimePageCommand command(type, args2, *socket);
        g_reloadRuntimePage = false;
        command.CheckAndRun();
        EXPECT_TRUE(g_reloadRuntimePage);

        std::string msg2 = "{\"ReloadRuntimePage\" : 222}";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg2);
        ReloadRuntimePageCommand command3(type, args3, *socket);
        g_reloadRuntimePage = false;
        command3.CheckAndRun();
        EXPECT_FALSE(g_reloadRuntimePage);
    }

    TEST_F(CommandLineTest, ToUint8Test)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        // null
        Json2::Value args1 = JsonReader::CreateNull();
        ResolutionSwitchCommand command1(type, args1, *socket);
        EXPECT_EQ(command1.ToUint8("256"), 0);
    }
 
    TEST_F(CommandLineTest, IsBoolTypeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        // null
        Json2::Value args1 = JsonReader::CreateNull();
        ResolutionSwitchCommand command1(type, args1, *socket);
        EXPECT_TRUE(command1.IsBoolType("true"));
        EXPECT_FALSE(command1.IsBoolType("XX"));
    }
 
    TEST_F(CommandLineTest, IsIntTypeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        // null
        Json2::Value args1 = JsonReader::CreateNull();
        ResolutionSwitchCommand command1(type, args1, *socket);
        EXPECT_TRUE(command1.IsIntType("123"));
    }
 
    TEST_F(CommandLineTest, IsOneDigitFloatTypeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        // null
        Json2::Value args1 = JsonReader::CreateNull();
        ResolutionSwitchCommand command1(type, args1, *socket);
        EXPECT_TRUE(command1.IsOneDigitFloatType("-6", true));
        EXPECT_TRUE(command1.IsOneDigitFloatType("3", false));
    }

    TEST_F(CommandLineTest, IsSetArgValidTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        // null
        Json2::Value args1 = JsonReader::CreateNull();
        ColorModeCommand command1(type, args1, *socket);
        command1.RunSet();
        EXPECT_FALSE(command1.IsSetArgValid());
    }

    TEST_F(CommandLineTest, FontSelectCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::CreateNull();
        FontSelectCommand command1(type, args1, *socket);
        g_output = false;
        command1.CheckAndRun();
        command1.RunSet();
        EXPECT_FALSE(command1.IsSetArgValid());
        EXPECT_TRUE(g_output);
        std::string msg = "{\"FontSelect\":true}";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg);
        FontSelectCommand command3(type, args2, *socket);
        g_output = false;
        command3.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, MemoryRefreshCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::CreateNull();
        MemoryRefreshCommand command1(type, args1, *socket);
        g_memoryRefresh = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_memoryRefresh);
        std::string msg = "{\"jsCode\":\"UEFOREEAAAAAAAAAAAAAA+wDAADEAAAAFQ\"}";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg);
        MemoryRefreshCommand command3(type, args2, *socket);
        g_memoryRefresh = false;
        command3.CheckAndRun();
        EXPECT_TRUE(g_memoryRefresh);
    }

    TEST_F(CommandLineTest, LoadDocumentCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args2 = JsonReader::CreateNull();
        LoadDocumentCommand command(type, args2, *socket);
        g_loadDocument = false;
        command.CheckAndRun();
        EXPECT_FALSE(g_loadDocument);
    }

    TEST_F(CommandLineTest, LoadDocumentCommandArgsTypeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg = R"({"url":"pages/Index","className":"Index","previewParam":{"width":1080,
            "height":"2340","locale":"zh_CN","colorMode":"light","orientation":"portrait",
            "deviceType":"phone","dpi":480}})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg);
        LoadDocumentCommand command(type, args2, *socket);
        g_loadDocument = false;
        command.CheckAndRun();
        EXPECT_FALSE(g_loadDocument);
    }

    TEST_F(CommandLineTest, LoadDocumentCommandArgsNumRangeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg = R"({"url" : "pages/Index", "className" : "Index", "previewParam" : {"width" : 1080,
            "height" : 2340, "locale" : "zh_CN" , "colorMode" : "light", "orientation" : "portrait",
            "deviceType" : "phone", "dpi" : 720}})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg);
        LoadDocumentCommand command(type, args2, *socket);
        g_loadDocument = false;
        command.CheckAndRun();
        EXPECT_FALSE(g_loadDocument);
    }

    TEST_F(CommandLineTest, LoadDocumentCommandArgsStrRangeTest)
    {
        CommandParser::GetInstance().deviceType = "phone";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg = R"({"url" : "pages/Index", "className" : "Index", "previewParam" : {"width" : 1080,
            "height" : 2340, "locale" : "aa_PP", "colorMode" : "light", "orientation" : "portrait",
            "deviceType" : "phone", "dpi" : 480}})";
        Json2::Value args = JsonReader::ParseJsonData2(msg);
        // locale error
        LoadDocumentCommand command(type, args, *socket);
        g_loadDocument = false;
        command.CheckAndRun();
        EXPECT_FALSE(g_loadDocument);
        // colorMode error
        std::string msg1 = R"({"url" : "pages/Index", "className" : "Index", "previewParam" : {"width" : 1080,
            "height" : 2340, "locale" : "zh_CN", "colorMode" : "aaa", "orientation" : "portrait",
            "deviceType" : "phone", "dpi" : 480}})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        LoadDocumentCommand command1(type, args1, *socket);
        g_loadDocument = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_loadDocument);
        // colorMode error
        std::string msg2 = R"({"url" : "pages/Index", "className" : "Index", "previewParam" : {"width" : 1080,
            "height" : 2340, "locale" : "zh_CN", "colorMode" : "dark", "orientation" : "aaa",
            "deviceType" : "phone", "dpi" : 480}})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        LoadDocumentCommand command2(type, args2, *socket);
        g_loadDocument = false;
        command2.CheckAndRun();
        EXPECT_FALSE(g_loadDocument);
        // deviceType error
        std::string msg3 = R"({"url" : "pages/Index", "className" : "Index", "previewParam" : {"width" : 1080,
            "height" : 2340, "locale" : "zh_CN", "colorMode" : "dark", "orientation" : "landscape",
            "deviceType" : "liteWearable", "dpi" : 480}})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg3);
        LoadDocumentCommand command3(type, args3, *socket);
        g_loadDocument = false;
        command3.CheckAndRun();
        EXPECT_FALSE(g_loadDocument);
    }

    TEST_F(CommandLineTest, LoadDocumentCommandArgsCorrectTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        CommandParser::GetInstance().deviceType = "phone";
        std::string msg = R"({"url":"pages/Index","className":"Index","previewParam":{"width":1080,
            "height":2340,"locale":"zh_CN","colorMode":"light","orientation":"portrait",
            "deviceType":"phone","dpi":480}})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg);
        LoadDocumentCommand command1(type, args1, *socket);
        g_loadDocument = false;
        command1.CheckAndRun();
        EXPECT_TRUE(g_loadDocument);

        CommandParser::GetInstance().deviceType = "liteWearable";
        msg = R"({"url":"pages/Index","className":"Index","previewParam":{"width":1080,
            "height":2340,"locale":"zh_CN","colorMode":"light","orientation":"portrait",
            "deviceType":"liteWearable","dpi":480}})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg);
        LoadDocumentCommand command2(type, args2, *socket);
        g_loadDocument = false;
        command2.CheckAndRun();
        EXPECT_FALSE(g_loadDocument);
    }

    TEST_F(CommandLineTest, FastPreviewMsgCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        FastPreviewMsgCommand command2(type, args2, *socket);
        g_getFastPreviewMsg = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_getFastPreviewMsg);
    }

    TEST_F(CommandLineTest, LoadContentCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        LoadContentCommand command2(type, args2, *socket);
        g_getAbilityCurrentRouter = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_getAbilityCurrentRouter);
    }

    TEST_F(CommandLineTest, DropFrameCommandTest)
    {
        VirtualScreenImpl::GetInstance().dropFrameFrequency = 0;
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg = R"({"frequency" : 1000})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg);
        DropFrameCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        EXPECT_EQ(VirtualScreenImpl::GetInstance().dropFrameFrequency, 1000); // set value is 1000

        VirtualScreenImpl::GetInstance().dropFrameFrequency = 0;
        std::string msg2 = R"({"frequency" : "aaaa"})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        DropFrameCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        EXPECT_EQ(VirtualScreenImpl::GetInstance().dropFrameFrequency, 0);

        VirtualScreenImpl::GetInstance().dropFrameFrequency = 0;
        std::string msg3 = R"({"frequency" : -100})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg3);
        DropFrameCommand command3(type, args3, *socket);
        command3.CheckAndRun();
        EXPECT_EQ(VirtualScreenImpl::GetInstance().dropFrameFrequency, 0);
    }

    TEST_F(CommandLineTest, KeyPressCommandImeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        int codePoint = 2033;
        std::string msg = R"({"isInputMethod":"aaa","codePoint":2033})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg);
        KeyPressCommand command1(type, args1, *socket);
        g_dispatchOsInputMethodEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsInputMethodEvent);

        args1.Replace("isInputMethod", true);
        args1.Replace("codePoint", "aaaa");
        KeyPressCommand command2(type, args1, *socket);
        g_dispatchOsInputMethodEvent = false;
        command2.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsInputMethodEvent);

        args1.Replace("codePoint", codePoint);
        KeyPressCommand command3(type, args1, *socket);
        g_dispatchOsInputMethodEvent = false;
        KeyInputImpl::GetInstance().codePoint = 0;
        command3.CheckAndRun();
        EXPECT_TRUE(g_dispatchOsInputMethodEvent);
        EXPECT_EQ(KeyInputImpl::GetInstance().codePoint, codePoint);
    }

    TEST_F(CommandLineTest, KeyPressCommandNoneImeArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg = R"({"isInputMethod":false,"keyCode":2033,"keyAction":0,"keyString":123,
            "pressedCodes":[2033]})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg);
        KeyPressCommand command1(type, args1, *socket);
        g_dispatchOsKeyEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsKeyEvent);
    }

    TEST_F(CommandLineTest, KeyPressCommandNoneImeArgsTypeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg = R"({"isInputMethod":false,"keyCode":2033,"keyAction":0,"pressedCodes":["aaa"]})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg);
        KeyPressCommand command1(type, args1, *socket);
        g_dispatchOsKeyEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsKeyEvent);
    }

    TEST_F(CommandLineTest, KeyPressCommandNoneImeArgsRangeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        // keyAction error
        std::string msg = R"({"isInputMethod" : false, "keyCode" : 2033, "keyAction" : 3, "keyString" : "123",
            "pressedCodes" : [2033]})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg);
        KeyPressCommand command1(type, args1, *socket);
        g_dispatchOsKeyEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsKeyEvent);
        // keyCode error
        args1.Replace("keyAction", 0);
        args1.Replace("keyCode", 1900);
        KeyPressCommand command2(type, args1, *socket);
        g_dispatchOsKeyEvent = false;
        command2.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsKeyEvent);
        // pressedCodes error
        msg = R"({"isInputMethod" : false, "keyCode" : 2033, "keyAction" : 1, "keyString" : "123",
            "pressedCodes" : [1900]})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg);
        KeyPressCommand command3(type, args2, *socket);
        g_dispatchOsKeyEvent = false;
        command3.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsKeyEvent);
    }

    TEST_F(CommandLineTest, KeyPressCommandNoneImeArgsCorrectTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"isInputMethod":false,"keyCode":2033,"keyAction":0,
            "keyString":"ctrl","pressedCodes":[2033]})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        KeyPressCommand command1(type, args1, *socket);
        g_dispatchOsKeyEvent = false;
        command1.CheckAndRun();
        EXPECT_TRUE(g_dispatchOsKeyEvent);
        EXPECT_EQ(KeyInputImpl::GetInstance().keyCode, 2033);
        EXPECT_EQ(KeyInputImpl::GetInstance().keyAction, 0);
        EXPECT_EQ(KeyInputImpl::GetInstance().keyString, "ctrl");
        EXPECT_EQ(KeyInputImpl::GetInstance().pressedCodes[0], OHOS::MMI::KeyCode(2033));
        
        std::string msg2 = R"({"isInputMethod":false,"keyCode":2033,"keyAction":0,"pressedCodes":[2033]})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        KeyPressCommand command2(type, args2, *socket);
        g_dispatchOsKeyEvent = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_dispatchOsKeyEvent);
        EXPECT_EQ(KeyInputImpl::GetInstance().keyString, "");

        KeyPressCommand command3(type, args1, *socket);
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::STATIC;
        g_dispatchOsKeyEvent = false;
        command3.CheckAndRun();
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::DYNAMIC;
        EXPECT_FALSE(g_dispatchOsKeyEvent);
    }

    TEST_F(CommandLineTest, PointEventCommandArgTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        // y error
        std::string msg1 = R"({"x":365,"y":"aaa","duration":"","button":1,"action": 2,"axisValues":[0,0,0,0],
            "sourceType":1,"sourceTool": 7,"pressedButtons":[0,1]})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        PointEventCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
        // action error
        args1.Replace("y", 1071);
        args1.Replace("action", "2");
        PointEventCommand command2(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command2.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
        // sourceTool error
        args1.Replace("action", 2);
        args1.Replace("sourceTool", "7");
        PointEventCommand command3(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command3.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
        // axisValues error
        args1.Replace("sourceTool", 7);
        args1.Replace("axisValues", "aaa");
        PointEventCommand command4(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command4.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, PointEventCommandArgRangeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        // x error
        std::string msg1 = R"({"x":2000,"y":1071,"duration":"","button":1,"action": 2,"axisValues":[0,0,0,0],
            "sourceType":1,"sourceTool": 7,"pressedButtons":[0,1]})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        PointEventCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
        // y error
        args1.Replace("x", 365);
        args1.Replace("y", 5000);
        PointEventCommand command2(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command2.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
        // sourceTool error
        args1.Replace("y", 1071);
        args1.Replace("sourceTool", -1);
        PointEventCommand command3(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command3.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
        // axisValues error
        msg1 = R"({"x" : 300, "y" : 1071, "duration" : "", "button" : 1, "action" : 2,
            "axisValues" : ["0", 0, 0, 0], "sourceType" : 1, "sourceTool" : 7, "pressedButtons" : [0, 1]})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg1);
        PointEventCommand command4(type, args2, *socket);
        g_dispatchOsTouchEvent = false;
        command4.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
        // pressedButtons errors
        msg1 = R"({"x" : 300, "y" : 1071, "duration" : "", "button" : 1, "action" : 2,
            "axisValues" : [0, 0, 0, 0], "sourceType" : 1, "sourceTool" : 7, "pressedButtons" : [-2, 0, 1]})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg1);
        PointEventCommand command5(type, args3, *socket);
        g_dispatchOsTouchEvent = false;
        command5.CheckAndRun();
        EXPECT_TRUE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, PointEventCommandArgCorrectTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"x":365,"y":1071,"duration":"","button":1,"action": 2,"axisValues":[0,0,0,0],
            "sourceType":1,"sourceTool": 7,"pressedButtons":[0,1]})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        PointEventCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_TRUE(g_dispatchOsTouchEvent);
        EXPECT_EQ(MouseInputImpl::GetInstance().mouseXPosition, 365); // 365 is test x
        EXPECT_EQ(MouseInputImpl::GetInstance().mouseYPosition, 1071); // 1071 is test y

        PointEventCommand command2(type, args1, *socket);
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::STATIC;
        g_dispatchOsTouchEvent = false;
        command2.CheckAndRun();
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::DYNAMIC;
        EXPECT_FALSE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, FoldStatusCommandArgsTest)
    {
        VirtualScreenImpl::GetInstance().SetFoldStatus("unfold");
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        // FoldStatus error
        std::string msg1 = R"({"FoldStatus":100,"width":1080,"height":2504})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        FoldStatusCommand command1(type, args1, *socket);
        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        command1.CheckAndRun();
        EXPECT_NE(JsAppImpl::GetInstance().width, 1080); // 1080 is test width
        EXPECT_NE(JsAppImpl::GetInstance().height, 2504); // 2504 is test height
        // height error
        args1.Replace("FoldStatus", "fold");
        args1.Replace("height", "aaa");
        FoldStatusCommand command2(type, args1, *socket);
        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        command2.CheckAndRun();
        EXPECT_NE(JsAppImpl::GetInstance().width, 1080); // 1080 is test width
        EXPECT_NE(JsAppImpl::GetInstance().height, 2504); // 2504 is test height
    }

    TEST_F(CommandLineTest, FoldStatusCommandArgsRangeTest)
    {
        VirtualScreenImpl::GetInstance().SetFoldStatus("unfold");
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        // FoldStatus error
        std::string msg1 = R"({"FoldStatus":"fold","width":1080,"height":4000})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        FoldStatusCommand command1(type, args1, *socket);
        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        command1.CheckAndRun();
        EXPECT_NE(JsAppImpl::GetInstance().width, 1080); // 1080 is test width
        EXPECT_NE(JsAppImpl::GetInstance().height, 4000); // 4000 is test height
        // height error
        args1.Replace("height", 2504); // 2504 is test height
        args1.Replace("FoldStatus", "aaaa");
        FoldStatusCommand command2(type, args1, *socket);
        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        command2.CheckAndRun();
        EXPECT_NE(JsAppImpl::GetInstance().width, 1080); // 1080 is test width
        EXPECT_NE(JsAppImpl::GetInstance().height, 2504); // 2504 is test height
    }

    TEST_F(CommandLineTest, FoldStatusCommandArgsCorrectTest)
    {
        VirtualScreenImpl::GetInstance().SetFoldStatus("unfold");
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg1 = R"({"FoldStatus":"fold","width":1080,"height":2504})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        FoldStatusCommand command1(type, args1, *socket);
        JsAppImpl::GetInstance().width = 0;
        JsAppImpl::GetInstance().height = 0;
        command1.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().width, 1080); // 1080 is test width
        EXPECT_EQ(JsAppImpl::GetInstance().height, 2504); // 2504 is test height
    }

    TEST_F(CommandLineTest, PowerCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg1 = R"({"Power":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        PowerCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        double power = SharedData<double>::GetData(SharedDataType::BATTERY_LEVEL);
        EXPECT_EQ(power, 1.0); // 1.0 is default Power value

        args1.Replace("Power", 2.0); // 2.0 is test Power value
        PowerCommand command2(type, args1, *socket);
        command2.CheckAndRun();
        power = SharedData<double>::GetData(SharedDataType::BATTERY_LEVEL);
        EXPECT_NE(power, 2.0); // 2.0 is test Power value

        args1.Replace("Power", -1);
        PowerCommand command3(type, args1, *socket);
        command3.CheckAndRun();
        power = SharedData<double>::GetData(SharedDataType::BATTERY_LEVEL);
        EXPECT_NE(power, -1); // -1 is test Power value
    }

    TEST_F(CommandLineTest, PowerCommandSetTest)
    {
        std::string msg1 = R"({"Power":0.5})";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        PowerCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        double power = SharedData<double>::GetData(SharedDataType::BATTERY_LEVEL);
        EXPECT_EQ(power, 0.5); // 0.5 is test Barometer value
    }

    TEST_F(CommandLineTest, PowerCommandGetTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        PowerCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, VolumeCommandTest)
    {
        std::string msg1 = R"({"Volume":90})";
        CommandLine::CommandType type1 = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        VolumeCommand command1(type1, args1, *socket);
        g_output = false;
        command1.CheckAndRun();
        EXPECT_TRUE(g_output);

        CommandLine::CommandType type2 = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        VolumeCommand command2(type2, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        command2.RunGet();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, BarometerCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg1 = R"({"Barometer":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        BarometerCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        int barometer = static_cast<int>(SharedData<uint32_t>::GetData(SharedDataType::PRESSURE_VALUE));
        EXPECT_EQ(barometer, 101325); // 101325 is default Power value

        args1.Replace("Barometer", 999901); // 999901 is test Power value
        BarometerCommand command2(type, args1, *socket);
        command2.CheckAndRun();
        barometer = static_cast<int>(SharedData<uint32_t>::GetData(SharedDataType::PRESSURE_VALUE));
        EXPECT_NE(barometer, 999901); // 999901 is test Power value

        args1.Replace("Barometer", -1);
        BarometerCommand command3(type, args1, *socket);
        command3.CheckAndRun();
        barometer = static_cast<int>(SharedData<uint32_t>::GetData(SharedDataType::PRESSURE_VALUE));
        EXPECT_NE(barometer, -1); // -1 is test Power value
    }

    TEST_F(CommandLineTest, BarometerCommandSetTest)
    {
        std::string msg1 = R"({"Barometer":999})";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        BarometerCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        int barometer = static_cast<int>(SharedData<uint32_t>::GetData(SharedDataType::PRESSURE_VALUE));
        EXPECT_EQ(barometer, 999); // 999 is test Barometer value
    }

    TEST_F(CommandLineTest, BarometerCommandGetTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        BarometerCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, LocationCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg0 = R"({"latitude":"10.0"})";
        Json2::Value args0 = JsonReader::ParseJsonData2(msg0);
        LocationCommand command0(type, args0, *socket);
        command0.CheckAndRun();
        double latitude = SharedData<double>::GetData(SharedDataType::LATITUDE);
        EXPECT_EQ(latitude, 0);
        std::string msg1 = R"({"latitude":"10.0","longitude":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        LocationCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        latitude = SharedData<double>::GetData(SharedDataType::LATITUDE);
        EXPECT_EQ(latitude, 0);
        args1.Replace("longitude", "10.0"); // 10.0 is test longitude value
        args1.Replace("latitude", "-91.0"); // -91 is test latitude value
        LocationCommand command2(type, args1, *socket);
        command2.CheckAndRun();
        latitude = SharedData<double>::GetData(SharedDataType::LATITUDE);
        EXPECT_NE(latitude, -91.0); // -91 is test longitude value
        args1.Replace("latitude", "91.0"); // 91 is test latitude value
        LocationCommand command3(type, args1, *socket);
        command3.CheckAndRun();
        latitude = SharedData<double>::GetData(SharedDataType::LATITUDE);
        EXPECT_NE(latitude, 91); // 91 is test longitude value
        args1.Replace("latitude", "10.0"); // 10.0 is test latitude value
        args1.Replace("longitude", "-181.0"); // -181 is test longitude value
        LocationCommand command4(type, args1, *socket);
        command4.CheckAndRun();
        double longitude = SharedData<double>::GetData(SharedDataType::LONGITUDE);
        EXPECT_NE(longitude, -181); // -181 is test longitude value
        args1.Replace("longitude", "181.0"); // 181 is test longitude value
        LocationCommand command5(type, args1, *socket);
        command5.CheckAndRun();
        longitude = SharedData<double>::GetData(SharedDataType::LONGITUDE);
        EXPECT_NE(longitude, 181); // 181 is test longitude value
    }

    TEST_F(CommandLineTest, LocationCommandSetTest)
    {
        std::string msg1 = R"({"latitude":"10.9023142","longitude":"56.3043242"})";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        LocationCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        double longitude = SharedData<double>::GetData(SharedDataType::LONGITUDE);
        double latitude = SharedData<double>::GetData(SharedDataType::LATITUDE);
        EXPECT_EQ(latitude, 10.9023142); // 10.9023142 is test longitude value
        EXPECT_EQ(longitude, 56.3043242); // 56.3043242 is test latitude value
    }

    TEST_F(CommandLineTest, LocationCommandGetTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        LocationCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, KeepScreenOnStateCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg0 = R"({"KeepScreenOnState111":false})";
        Json2::Value args0 = JsonReader::ParseJsonData2(msg0);
        KeepScreenOnStateCommand command0(type, args0, *socket);
        command0.CheckAndRun();
        bool status = SharedData<bool>::GetData(SharedDataType::KEEP_SCREEN_ON);
        EXPECT_EQ(status, true);

        std::string msg1 = R"({"KeepScreenOnState":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        KeepScreenOnStateCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        status = SharedData<bool>::GetData(SharedDataType::KEEP_SCREEN_ON);
        EXPECT_EQ(status, true);
    }

    TEST_F(CommandLineTest, KeepScreenOnStateCommandSetTest)
    {
        std::string msg1 = R"({"KeepScreenOnState":false})";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        KeepScreenOnStateCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        bool status = SharedData<bool>::GetData(SharedDataType::KEEP_SCREEN_ON);
        EXPECT_EQ(status, false);
    }

    TEST_F(CommandLineTest, KeepScreenOnStateCommandGetTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        KeepScreenOnStateCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, WearingStateCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg0 = R"({"WearingState11":false})";
        Json2::Value args0 = JsonReader::ParseJsonData2(msg0);
        WearingStateCommand command0(type, args0, *socket);
        command0.CheckAndRun();
        bool status = SharedData<bool>::GetData(SharedDataType::WEARING_STATE);
        EXPECT_EQ(status, true);

        std::string msg1 = R"({"WearingState":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        WearingStateCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        status = SharedData<bool>::GetData(SharedDataType::WEARING_STATE);
        EXPECT_EQ(status, true);
    }

    TEST_F(CommandLineTest, WearingStateCommandSetTest)
    {
        std::string msg1 = R"({"WearingState":false})";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        WearingStateCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        bool status = SharedData<bool>::GetData(SharedDataType::WEARING_STATE);
        EXPECT_EQ(status, false);
    }

    TEST_F(CommandLineTest, WearingStateCommandGetTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        WearingStateCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, BrightnessModeCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg0 = R"({"BrightnessMode111":1})";
        Json2::Value args0 = JsonReader::ParseJsonData2(msg0);
        BrightnessModeCommand command0(type, args0, *socket);
        command0.CheckAndRun();
        uint8_t brightness = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_MODE);
        EXPECT_EQ(brightness, 0); // 0 is default brightness

        std::string msg1 = R"({"BrightnessMode":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        BrightnessModeCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        brightness = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_MODE);
        EXPECT_EQ(brightness, 0); // 0 is default brightness

        msg1 = R"({"BrightnessMode":-1})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg1);
        BrightnessModeCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        brightness = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_MODE);
        EXPECT_NE(brightness, -1); // -1 is test brightness value

        msg1 = R"({"BrightnessMode":2})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg1);
        BrightnessModeCommand command3(type, args3, *socket);
        command3.CheckAndRun();
        brightness = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_MODE);
        EXPECT_NE(brightness, 2); // 2 is test brightness value
    }

    TEST_F(CommandLineTest, BrightnessModeCommandSetTest)
    {
        std::string msg1 = R"({"BrightnessMode":1})";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        BrightnessModeCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        uint8_t brightness = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_MODE);
        EXPECT_EQ(brightness, 1); // 1 is default brightness
    }

    TEST_F(CommandLineTest, BrightnessModeCommandGetTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        BrightnessModeCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, ChargeModeCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg0 = R"({"ChargeMode111":1})";
        Json2::Value args0 = JsonReader::ParseJsonData2(msg0);
        ChargeModeCommand command0(type, args0, *socket);
        command0.CheckAndRun();
        uint8_t mode = SharedData<uint8_t>::GetData(SharedDataType::BATTERY_STATUS);
        EXPECT_EQ(mode, 0); // 0 is default mode

        std::string msg1 = R"({"ChargeMode":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        ChargeModeCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        mode = SharedData<uint8_t>::GetData(SharedDataType::BATTERY_STATUS);
        EXPECT_EQ(mode, 0); // 0 is default mode

        msg1 = R"({"ChargeMode":-1})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg1);
        ChargeModeCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        mode = SharedData<uint8_t>::GetData(SharedDataType::BATTERY_STATUS);
        EXPECT_NE(mode, -1); // -1 is test mode value

        msg1 = R"({"ChargeMode":2})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg1);
        ChargeModeCommand command3(type, args3, *socket);
        command3.CheckAndRun();
        mode = SharedData<uint8_t>::GetData(SharedDataType::BATTERY_STATUS);
        EXPECT_NE(mode, 2); // 2 is test mode value
    }

    TEST_F(CommandLineTest, ChargeModeCommandSetTest)
    {
        std::string msg1 = R"({"ChargeMode":1})";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        ChargeModeCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        uint8_t mode = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_MODE);
        EXPECT_EQ(mode, 1); // 1 is default mode
    }

    TEST_F(CommandLineTest, ChargeModeCommandGetTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        ChargeModeCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, BrightnessCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg0 = R"({"Brightness111":100})";
        Json2::Value args0 = JsonReader::ParseJsonData2(msg0);
        BrightnessCommand command0(type, args0, *socket);
        command0.CheckAndRun();
        uint8_t mode = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_VALUE);
        EXPECT_EQ(mode, 255); // 255 is default mode

        std::string msg1 = R"({"Brightness":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        BrightnessCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        mode = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_VALUE);
        EXPECT_EQ(mode, 255); // 255 is default mode

        msg1 = R"({"Brightness":256})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg1);
        BrightnessCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        mode = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_VALUE);
        EXPECT_NE(mode, 256); // 256 is test mode value

        msg1 = R"({"Brightness":0})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg1);
        BrightnessCommand command3(type, args3, *socket);
        command3.CheckAndRun();
        mode = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_VALUE);
        EXPECT_NE(mode, 0); // 0 is test mode value
    }

    TEST_F(CommandLineTest, BrightnessCommandSetTest)
    {
        std::string msg1 = R"({"Brightness":100})";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        BrightnessCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        uint8_t mode = SharedData<uint8_t>::GetData(SharedDataType::BRIGHTNESS_VALUE);
        EXPECT_EQ(mode, 100); // 100 is test mode
    }

    TEST_F(CommandLineTest, BrightnessCommandGetTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        BrightnessCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, HeartRateCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg0 = R"({"HeartRate111":100})";
        Json2::Value args0 = JsonReader::ParseJsonData2(msg0);
        HeartRateCommand command0(type, args0, *socket);
        command0.CheckAndRun();
        uint8_t mode = SharedData<uint8_t>::GetData(SharedDataType::HEARTBEAT_VALUE);
        EXPECT_EQ(mode, 80); // 80 is default mode

        std::string msg1 = R"({"HeartRate":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        HeartRateCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        mode = SharedData<uint8_t>::GetData(SharedDataType::HEARTBEAT_VALUE);
        EXPECT_EQ(mode, 80); // 80 is default mode

        msg1 = R"({"HeartRate":256})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg1);
        HeartRateCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        mode = SharedData<uint8_t>::GetData(SharedDataType::HEARTBEAT_VALUE);
        EXPECT_NE(mode, 256); // 256 is test mode value

        msg1 = R"({"HeartRate":-1})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg1);
        HeartRateCommand command3(type, args3, *socket);
        command3.CheckAndRun();
        mode = SharedData<uint8_t>::GetData(SharedDataType::HEARTBEAT_VALUE);
        EXPECT_NE(mode, -1); // -1 is test mode value
    }

    TEST_F(CommandLineTest, HeartRateCommandSetTest)
    {
        std::string msg1 = R"({"HeartRate":100})";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        HeartRateCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        uint8_t mode = SharedData<uint8_t>::GetData(SharedDataType::HEARTBEAT_VALUE);
        EXPECT_EQ(mode, 100); // 100 is test mode
    }

    TEST_F(CommandLineTest, HeartRateCommandGetTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        HeartRateCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, StepCountCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg0 = R"({"StepCount111":100})";
        Json2::Value args0 = JsonReader::ParseJsonData2(msg0);
        StepCountCommand command0(type, args0, *socket);
        command0.CheckAndRun();
        uint32_t mode = SharedData<uint32_t>::GetData(SharedDataType::SUMSTEP_VALUE);
        EXPECT_EQ(mode, 0); // 0 is default mode

        std::string msg1 = R"({"StepCount":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        StepCountCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        mode = SharedData<uint32_t>::GetData(SharedDataType::SUMSTEP_VALUE);
        EXPECT_EQ(mode, 0); // 0 is default mode

        msg1 = R"({"StepCount":10000000})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg1);
        StepCountCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        mode = SharedData<uint32_t>::GetData(SharedDataType::SUMSTEP_VALUE);
        EXPECT_NE(mode, 10000000); // 10000000 is test mode value

        msg1 = R"({"StepCount":-1})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg1);
        StepCountCommand command3(type, args3, *socket);
        command3.CheckAndRun();
        mode = SharedData<uint32_t>::GetData(SharedDataType::SUMSTEP_VALUE);
        EXPECT_NE(mode, -1); // -1 is test mode value
    }

    TEST_F(CommandLineTest, StepCountCommandSetTest)
    {
        std::string msg1 = R"({"StepCount":100})";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        StepCountCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        uint8_t mode = SharedData<uint32_t>::GetData(SharedDataType::SUMSTEP_VALUE);
        EXPECT_EQ(mode, 100); // 100 is test mode
    }

    TEST_F(CommandLineTest, StepCountCommandGetTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        StepCountCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, DistributedCommunicationsCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"DeviceId":"68-05-CA-90-9A-66","bundleName":"abc",
            "abilityName":"hello"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        DistributedCommunicationsCommand command1(type, args1, *socket);
        g_sendVirtualMessage = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_sendVirtualMessage);

        std::string msg2 = R"({"DeviceId":"68-05-CA-90-9A-66","bundleName":"abc",
            "abilityName":"hello"},"message":"")";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        DistributedCommunicationsCommand command2(type, args2, *socket);
        g_sendVirtualMessage = false;
        command2.CheckAndRun();
        EXPECT_FALSE(g_sendVirtualMessage);

        std::string msg3 = R"({"DeviceId":"68-05-CA-90-9A-66","bundleName":"abc",
            "abilityName":"hello","message":"{ action:'GET_WEATHER',city:'HangZhou' }"})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg3);
        DistributedCommunicationsCommand command3(type, args3, *socket);
        g_sendVirtualMessage = false;
        command3.CheckAndRun();
        EXPECT_TRUE(g_sendVirtualMessage);

        std::string msg4 = R"({"DeviceId" : "68-05-CA-90-9A-66", "bundleName" : "abc",
            "abilityName" : "hello", "message" : ""})";
        Json2::Value args4 = JsonReader::ParseJsonData2(msg4);
        DistributedCommunicationsCommand command4(type, args4, *socket);
        g_sendVirtualMessage = false;
        command4.CheckAndRun();
        EXPECT_FALSE(g_sendVirtualMessage);
    }

    TEST_F(CommandLineTest, DistributedCommunicationsCommandTest2)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg = R"({"DeviceId" : "68-05-CA-90-9A-66", "bundleName" : "abc",
            "abilityName" : "hello", "message" : "{ action : 'GET_WEATHER', city : 'HangZhou' }"})";
        Json2::Value args = JsonReader::ParseJsonData2(msg);
        DistributedCommunicationsCommand command(type, args, *socket);
        std::vector<char> vec = command.StringToCharVector("123");
        int size = 4;
        EXPECT_EQ(vec.size(), size);
    }

    TEST_F(CommandLineTest, MouseWheelCommandTest)
    {
        std::string msg1 = R"({"rotate":"aaa"})";
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        MouseWheelCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        EXPECT_EQ(MouseWheelImpl::GetInstance().rotate, 0); // 0 is default value

        std::string msg2 = R"({"rotate":100})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        MouseWheelCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        EXPECT_EQ(MouseWheelImpl::GetInstance().GetRotate(), 100); // 100 is test mode

        msg2 = R"({"rotate":150})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg2);
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::STATIC;
        MouseWheelCommand command3(type, args3, *socket);
        command3.CheckAndRun();
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::DYNAMIC;
        EXPECT_NE(MouseWheelImpl::GetInstance().GetRotate(), 150); // 100 is test mode
    }

    TEST_F(CommandLineTest, TouchPressCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"x":365,"y":"abc","duration":""})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        TouchPressCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, TouchPressCommandArgsRangeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"x":365,"y":15000,"duration":""})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        TouchPressCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);

        std::string msg2 = R"({"x":-1,"y":15000,"duration":""})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        TouchPressCommand command2(type, args2, *socket);
        g_dispatchOsTouchEvent = false;
        command2.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);

        std::string msg3 = R"({"x":15000,"y":365,"duration":""})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg3);
        TouchPressCommand command3(type, args3, *socket);
        g_dispatchOsTouchEvent = false;
        command3.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);

        std::string msg4 = R"({"x":15000,"y":-1,"duration":""})";
        Json2::Value args4 = JsonReader::ParseJsonData2(msg4);
        TouchPressCommand command4(type, args4, *socket);
        g_dispatchOsTouchEvent = false;
        command4.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, TouchPressCommandArgsCorrectTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"x":365,"y":1076,"duration":""})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        TouchPressCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_TRUE(g_dispatchOsTouchEvent);

        TouchPressCommand command2(type, args1, *socket);
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::STATIC;
        g_dispatchOsTouchEvent = false;
        command2.CheckAndRun();
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::DYNAMIC;
        EXPECT_FALSE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, TouchReleaseCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"x":365,"y":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        TouchReleaseCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, TouchReleaseCommandArgsRangeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"x":365,"y":15000})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        TouchReleaseCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);

        std::string msg2 = R"({"x":-1,"y":15000})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        TouchReleaseCommand command2(type, args2, *socket);
        g_dispatchOsTouchEvent = false;
        command2.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);

        std::string msg3 = R"({"x":15000,"y":365})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg3);
        TouchReleaseCommand command3(type, args3, *socket);
        g_dispatchOsTouchEvent = false;
        command3.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);

        std::string msg4 = R"({"x":15000,"y":-1})";
        Json2::Value args4 = JsonReader::ParseJsonData2(msg4);
        TouchReleaseCommand command4(type, args4, *socket);
        g_dispatchOsTouchEvent = false;
        command4.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, TouchReleaseCommandArgsCorrectTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"x":365,"y":1076})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        TouchReleaseCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_TRUE(g_dispatchOsTouchEvent);

        TouchReleaseCommand command2(type, args1, *socket);
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::STATIC;
        g_dispatchOsTouchEvent = false;
        command2.CheckAndRun();
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::DYNAMIC;
        EXPECT_FALSE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, TouchMoveCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"x":365,"y":"abc"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        TouchMoveCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, TouchMoveCommandArgsRangeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"x":365,"y":15000})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        TouchMoveCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);

        std::string msg2 = R"({"x":-1,"y":15000})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        TouchMoveCommand command2(type, args2, *socket);
        g_dispatchOsTouchEvent = false;
        command2.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);

        std::string msg3 = R"({"x":15000,"y":365})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg3);
        TouchMoveCommand command3(type, args3, *socket);
        g_dispatchOsTouchEvent = false;
        command3.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);

        std::string msg4 = R"({"x":15000,"y":-1})";
        Json2::Value args4 = JsonReader::ParseJsonData2(msg4);
        TouchMoveCommand command4(type, args4, *socket);
        g_dispatchOsTouchEvent = false;
        command4.CheckAndRun();
        EXPECT_FALSE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, TouchMoveCommandArgsCorrectTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        std::string msg1 = R"({"x":365,"y":1076})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        TouchMoveCommand command1(type, args1, *socket);
        g_dispatchOsTouchEvent = false;
        command1.CheckAndRun();
        EXPECT_TRUE(g_dispatchOsTouchEvent);

        TouchMoveCommand command2(type, args1, *socket);
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::STATIC;
        g_dispatchOsTouchEvent = false;
        command2.CheckAndRun();
        CommandParser::GetInstance().screenMode = CommandParser::ScreenMode::DYNAMIC;
        EXPECT_FALSE(g_dispatchOsTouchEvent);
    }

    TEST_F(CommandLineTest, LanguageCommandArgsTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg1 = R"({"Language":111})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        LanguageCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        std::string language = SharedData<string>::GetData(SharedDataType::LANGUAGE);
        EXPECT_EQ(language, "zh-CN"); // zh-CN is default value

        CommandParser::GetInstance().deviceType = "liteWearable";
        std::string msg2 = R"({"Language":"ar_AE"})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        LanguageCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        language = SharedData<string>::GetData(SharedDataType::LANGUAGE);
        EXPECT_NE(language, "ar_AE");

        CommandParser::GetInstance().deviceType = "phone";
        std::string msg3 = R"({"Language":"aa_BB"})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg3);
        LanguageCommand command3(type, args3, *socket);
        command3.CheckAndRun();
        language = SharedData<string>::GetData(SharedDataType::LANGUAGE);
        EXPECT_NE(language, "aa_BB");
    }

    TEST_F(CommandLineTest, LanguageCommandSetTest)
    {
        CommandParser::GetInstance().deviceType = "liteWearable";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg1 = R"({"Language":"en-US"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        LanguageCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        std::string language = SharedData<string>::GetData(SharedDataType::LANGUAGE);
        EXPECT_EQ(language, "en-US");

        CommandParser::GetInstance().deviceType = "phone";
        std::string msg2 = R"({"Language":"en_US"})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        LanguageCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        language = SharedData<string>::GetData(SharedDataType::LANGUAGE);
        EXPECT_EQ(language, "en_US");
    }

    TEST_F(CommandLineTest, LanguageCommandGetTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        LanguageCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, SupportedLanguagesCommandTest)
    {
        CommandParser::GetInstance().deviceType = "liteWearable";
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args1 = JsonReader::CreateNull();
        SupportedLanguagesCommand command1(type, args1, *socket);
        g_output = false;
        command1.CheckAndRun();
        EXPECT_TRUE(g_output);

        CommandParser::GetInstance().deviceType = "phone";
        SupportedLanguagesCommand command2(type, args1, *socket);
        command2.CheckAndRun();
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, ExitCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::ACTION;
        Interrupter::isInterrupt = false;
        Json2::Value args1 = JsonReader::CreateNull();
        ExitCommand command1(type, args1, *socket);
        g_output = false;
        command1.CheckAndRun();
        EXPECT_TRUE(Interrupter::isInterrupt);
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, RestartCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        RestartCommand command2(type, args2, *socket);
        command2.RunAction();
    }

    TEST_F(CommandLineTest, ResolutionCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::CreateNull();
        ResolutionCommand command1(type, args1, *socket);
        g_output = false;
        command1.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, DeviceTypeCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args1 = JsonReader::CreateNull();
        DeviceTypeCommand command1(type, args1, *socket);
        g_output = false;
        command1.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, AvoidAreaCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg1 = R"({"topRect":{"posX":0,"posY":0,"width":2340,"height":117},"bottomRect":
            {"bottomRect":0,"posY":0,"width":0,"height":0},"leftRect":{"posX":0,"posY":0,"width":0,"height":0}})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        AvoidAreaCommand command1(type, args1, *socket);
        g_output = false;
        command1.CheckAndRun();
        command1.RunSet();
        EXPECT_TRUE(g_output);
        std::string msg2 = R"({"topRect":{"posX":0,"posY":0,"width":2340,"height":117},"bottomRect":{"bottomRect":
            0,"posY":0,"width":0,"height":0},"leftRect":{"posX":0,"posY":0,"width":0,"height":0},"rightRect":0})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        AvoidAreaCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
        std::string msg3 = R"({"topRect":{"posX":0,"posY":0,"width":2340,"height":117},"bottomRect":
            {"bottomRect":0,"posY":0,"width":0,"height":0},"leftRect":{"posX":0,"posY":0,"width":0,"height":0},
            "rightRect":{"posX":0,"posY":0,"width":0}})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg3);
        AvoidAreaCommand command3(type, args3, *socket);
        g_output = false;
        command3.CheckAndRun();
        EXPECT_TRUE(g_output);
        std::string msg4 = R"({"topRect":{"posX":0,"posY":0,"width":2340,"height":117},"bottomRect":
            {"bottomRect":0,"posY":0,"width":0,"height":0},"leftRect":{"posX":0,"posY":0,"width":0,"height":0},
            "rightRect":{"posX":0,"posY":0,"width":0,"height":"350"}})";
        Json2::Value args4 = JsonReader::ParseJsonData2(msg4);
        AvoidAreaCommand command4(type, args4, *socket);
        g_output = false;
        command4.CheckAndRun();
        EXPECT_TRUE(g_output);
        std::string msg5 = R"({"topRect":{"posX":0,"posY":0,"width":2340,"height":117},"bottomRect":{"bottomRect":
            0,"posY":0,"width":0,"height":0},"leftRect":{"posX":0,"posY":0,"width":0,"height":0},
            "rightRect":{"posX":0,"posY":0,"width":-1,"height":-1}})";
        Json2::Value args5 = JsonReader::ParseJsonData2(msg5);
        AvoidAreaCommand command5(type, args5, *socket);
        g_output = false;
        command5.CheckAndRun();
        EXPECT_TRUE(g_output);
        std::string msg6 = R"({"topRect":{"posX":0,"posY":0,"width":2340,"height":117},"bottomRect":{"bottomRect":
            0,"posY":0,"width":0,"height":0},"leftRect":{"posX":0,"posY":0,"width":0,"height":0},
            "rightRect":{"posX":0,"posY":0,"width":2340,"height":84}})";
        Json2::Value args6 = JsonReader::ParseJsonData2(msg6);
        AvoidAreaCommand command6(type, args6, *socket);
        g_output = false;
        command6.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, AvoidAreaCommandArgsRangeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg1 = R"({"topRect" : {"posX"  :0, "posY" : 0, "width" : 2340, "height" : 117},
            "bottomRect" : {"posX" : 0, "posY" : 0, "width" : 0, "height" : 0}, "leftRect" : {"posX" : 0,
            "posY" : 0, "width" : 0, "height" : 0}, "rightRect" : {"posX" : 0, "posY" : 0, "width" : 2340,
            "height" : "84"}})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        AvoidAreaCommand command1(type, args1, *socket);
        g_output = false;
        command1.CheckAndRun();
        EXPECT_TRUE(g_output);

        std::string msg2 = R"({"topRect" : {"posX"  :0, "posY" : 0, "width" : 2340, "height" : 117},
            "bottomRect" : {"posX" : 0, "posY" : 0, "width" : 0, "height" : 0}, "leftRect" : {"posX" : 0,
            "posY" : 0, "width" : 0, "height" : 0}, "rightRect" : {"posX" : 0, "posY" : -2, "width" : 2340,
            "height" : 84}})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        AvoidAreaCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);


        std::string msg3 = R"({"topRect" : {"posX"  :0, "posY" : 0, "width" : 2340, "height" : 117},
            "bottomRect" : {"posX" : 0, "posY" : 0, "width" : 0, "height" : 0}, "leftRect" : {"posX" : 0,
            "posY" : 0, "width" : 0, "height" : 0}, "rightRect" : {"posX" : 0, "posY" : 0, "width" : 2340,
            "height" : 84}})";
        Json2::Value args3 = JsonReader::ParseJsonData2(msg3);
        AvoidAreaCommand command3(type, args3, *socket);
        g_output = false;
        command3.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, AvoidAreaChangedCommandTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args2 = JsonReader::CreateNull();
        AvoidAreaChangedCommand command2(type, args2, *socket);
        g_output = false;
        command2.CheckAndRun();
        EXPECT_TRUE(g_output);
    }

    TEST_F(CommandLineTest, IsArgValidTest_Err)
    {
        CommandLine::CommandType type = CommandLine::CommandType::GET;
        Json2::Value args1 = JsonReader::CreateObject();
        CurrentRouterCommand command2(type, args1, *socket);
        command2.type = CommandLine::CommandType::INVALID;
        EXPECT_TRUE(command2.IsArgValid());
    }

    TEST_F(CommandLineTest, ColorModeCommandArgsCorrectTest)
    {
        JsAppImpl::GetInstance().colorMode = "light";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        std::string msg = R"({"ColorMode" : "dark"})";
        Json2::Value args = JsonReader::ParseJsonData2(msg);
        ColorModeCommand command(type, args, *socket);
        command.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().colorMode, "dark");
    }

    TEST_F(CommandLineTest, ColorModeCommandArgsTypeTest)
    {
        JsAppImpl::GetInstance().colorMode = "light";
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        Json2::Value args = JsonReader::CreateNull();
        ColorModeCommand command(type, args, *socket);
        command.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().colorMode, "light");

        JsAppImpl::GetInstance().colorMode = "light";
        std::string msg1 = R"({"aaa" : "dark"})";
        Json2::Value args1 = JsonReader::ParseJsonData2(msg1);
        ColorModeCommand command1(type, args1, *socket);
        command1.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().colorMode, "light");

        JsAppImpl::GetInstance().colorMode = "light";
        std::string msg2 = R"({"ColorMode" : 123})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        ColorModeCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().colorMode, "light");
    }

    TEST_F(CommandLineTest, ColorModeCommandArgsRangeTest)
    {
        CommandLine::CommandType type = CommandLine::CommandType::SET;
        JsAppImpl::GetInstance().colorMode = "light";
        std::string msg2 = R"({"ColorMode" : "aaa"})";
        Json2::Value args2 = JsonReader::ParseJsonData2(msg2);
        ColorModeCommand command2(type, args2, *socket);
        command2.CheckAndRun();
        EXPECT_EQ(JsAppImpl::GetInstance().colorMode, "light");
    }
}
