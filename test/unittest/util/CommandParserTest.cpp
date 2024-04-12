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
#include <sstream>
#include <vector>
#include <fstream>
#include <cstdio>
#include <unistd.h>
#include "gtest/gtest.h"
#define private public
#include "CommandParser.h"
using namespace std;

namespace {
    class CommandParserTest : public ::testing::Test {
    public:
        CommandParserTest() {}
        ~CommandParserTest() {}
    protected:
        // 在整个测试夹具类执行前执行一次初始化操作
        static void SetUpTestCase()
        {
            std::stringstream ss1(invalidParams);
            std::string invalidParam;
            // 使用空格分割字符串，并存入vector
            char buffer[FILENAME_MAX];
            if (getcwd(buffer, FILENAME_MAX) != nullptr) {
                currDir = std::string(buffer);
                currFile = currDir + "/mytestfilename";
                // 创建文件流对象并打开文件
                std::ofstream file(currFile);
                // 检查文件是否成功打开
                if (file.is_open()) {
                    file.close();
                } else {
                    printf("Error creating file!\n");
                }
            } else {
                printf("error: getcwd failed\n");
            }
            while (std::getline(ss1, invalidParam, ' ')) {
                invalidParamVec.push_back(invalidParam);
                if (invalidParam == "=dir=") {
                    invalidParam = currDir;
                } else if (invalidParam == "=file=") {
                    invalidParam = currFile;
                } else if (invalidParam == "-h") {
                    continue; // 不存在的参数存储
                }
                validParamVec.push_back(invalidParam);
            }
        }
        // 在整个测试夹具类执行后执行一次清理操作
        static void TearDownTestCase()
        {
            if (std::remove(currFile.c_str()) != 0) {
                printf("Error deleting file!\n");
            }
        }

        static std::vector<std::string> invalidParamVec;
        static std::vector<std::string> validParamVec;
        static std::string invalidParams;
        static std::string currDir;
        static std::string currFile;
    };
    std::string CommandParserTest::currDir = "";
    std::string CommandParserTest::currFile = "";
    std::vector<std::string> CommandParserTest::invalidParamVec = {};
    std::vector<std::string> CommandParserTest::validParamVec = {};
    std::string CommandParserTest::invalidParams = "-refresh region "
        "-projectID 138968279 "
        "-ts trace_70259_commandPipe "
        "-j =dir= "
        "-s componentpreviewinstance_1712054594321_1 "
        "-cpm true "
        "-device phone "
        "-shape rect "
        "-sd 480 "
        "-ljPath =file= "
        "-or 1080 2340 "
        "-cr 1080 2340 "
        "-fr 1080 2504 "
        "-f =file= "
        "-n entry "
        "-av ACE_2_0 "
        "-url pages/BigImg_1M "
        "-pages main_pages "
        "-arp =dir= "
        "-pm Stage "
        "-hsp =dir= "
        "-l zh_CN "
        "-cm light "
        "-o portrait "
        "-lws 40003 "
        "-card true "
        "-p 8888 "
        "-abn EntryAbility "
        "-abp ets/entryability/EntryAbility.abc "
        "-d "
        "-sm static "
        "-staticCard true "
        "-foldable true "
        "-foldStatus half_fold "
        "-hs 100000 "
        "-hf true "
        "-h";

    TEST_F(CommandParserTest, IsSetTest)
    {
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsSet("arp"));
        EXPECT_FALSE(CommandParser::GetInstance().IsSet("abc"));
    }

    TEST_F(CommandParserTest, IsCommandValidTest1)
    {
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
    }

    TEST_F(CommandParserTest, IsCommandValidTest2)
    {
        EXPECT_FALSE(CommandParser::GetInstance().ProcessCommand(invalidParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
    }

    TEST_F(CommandParserTest, ValueTest)
    {
        string str1 = "abcd";
        string str2 = "rect";
        string str3 = CommandParser::GetInstance().Value("shape");
        EXPECT_EQ(str2, str3);
        EXPECT_NE(str1, str3);
    }

    TEST_F(CommandParserTest, ValuesTest)
    {
        std::vector<std::string> vec1 = {"1111", "2222"};
        std::vector<std::string> vec2 = {"1080", "2340"};
        std::vector<std::string> vec3 = CommandParser::GetInstance().Values("or");
        for (int i = 0; i < vec3.size(); i++) {
            EXPECT_NE(vec1[i], vec3[i]);
        }
        for (int i = 0; i < vec3.size(); i++) {
            EXPECT_EQ(vec2[i], vec3[i]);
        }
    }

    TEST_F(CommandParserTest, RegisterTest)
    {
        std::string key = "-abcd";
        uint32_t count = 1;
        CommandParser::GetInstance().Register(key, count, "abcd");
        uint32_t num = CommandParser::GetInstance().regsArgsCountMap[key];
        string helpStr = CommandParser::GetInstance().regsHelpMap[key];
        EXPECT_EQ(num, count);
        EXPECT_EQ(helpStr, "abcd");
    }

    TEST_F(CommandParserTest, IsResolutionValidTest)
    {
        int32_t val1 = 0;
        int32_t val2 = 1;
        int32_t val3 = 1024;
        int32_t val4 = 3840;
        int32_t val5 = 3841;
        EXPECT_FALSE(CommandParser::GetInstance().IsResolutionValid(val1));
        EXPECT_TRUE(CommandParser::GetInstance().IsResolutionValid(val2));
        EXPECT_TRUE(CommandParser::GetInstance().IsResolutionValid(val3));
        EXPECT_TRUE(CommandParser::GetInstance().IsResolutionValid(val4));
        EXPECT_FALSE(CommandParser::GetInstance().IsResolutionValid(val5));
    }

    TEST_F(CommandParserTest, GetOrignalResolutionWidthTest)
    {
        int32_t val1 = 1080;
        EXPECT_EQ(CommandParser::GetInstance().GetOrignalResolutionWidth(), val1);
    }

    TEST_F(CommandParserTest, GetOrignalResolutionHeightTest)
    {
        int32_t val1 = 2340;
        EXPECT_EQ(CommandParser::GetInstance().GetOrignalResolutionHeight(), val1);
    }

    TEST_F(CommandParserTest, GetCompressionResolutionWidthTest)
    {
        int32_t val1 = 1080;
        EXPECT_EQ(CommandParser::GetInstance().GetCompressionResolutionWidth(), val1);
    }

    TEST_F(CommandParserTest, GetCompressionResolutionHeightTest)
    {
        int32_t val1 = 2340;
        EXPECT_EQ(CommandParser::GetInstance().GetCompressionResolutionHeight(), val1);
    }

    TEST_F(CommandParserTest, GetJsHeapSizeTest)
    {
        int32_t val1 = 100000;
        EXPECT_EQ(CommandParser::GetInstance().GetJsHeapSize(), val1);
    }

    TEST_F(CommandParserTest, GetAppNameTest)
    {
        std::string val1 = "entry";
        EXPECT_EQ(CommandParser::GetInstance().GetAppName(), val1);
    }

    TEST_F(CommandParserTest, IsSendJSHeapTest)
    {
        EXPECT_TRUE(CommandParser::GetInstance().IsSendJSHeap());
    }
    
    TEST_F(CommandParserTest, GetDeviceTypeTest)
    {
        EXPECT_EQ(CommandParser::GetInstance().GetDeviceType(), "phone");
    }

    TEST_F(CommandParserTest, IsRegionRefreshTest)
    {
        EXPECT_TRUE(CommandParser::GetInstance().IsRegionRefresh());
    }

    TEST_F(CommandParserTest, IsCardDisplayTest)
    {
        EXPECT_TRUE(CommandParser::GetInstance().IsCardDisplay());
    }

    TEST_F(CommandParserTest, GetConfigPathTest)
    {
        EXPECT_EQ(CommandParser::GetInstance().GetConfigPath(), currFile);
    }

    TEST_F(CommandParserTest, GetProjectIDTest)
    {
        std::string str = "138968279";
        EXPECT_EQ(CommandParser::GetInstance().GetProjectID(), str);
    }

    TEST_F(CommandParserTest, GetScreenModeTest)
    {
        std::string str = "static";
        EXPECT_EQ(CommandParser::GetInstance().GetScreenMode(), CommandParser::ScreenMode::STATIC);
    }

    TEST_F(CommandParserTest, GetConfigChangesTest)
    {
        EXPECT_EQ(CommandParser::GetInstance().GetConfigChanges(), "");
    }

    TEST_F(CommandParserTest, GetAppResourcePathTest)
    {
        EXPECT_EQ(CommandParser::GetInstance().GetAppResourcePath(), currDir);
    }

    TEST_F(CommandParserTest, GetScreenShapeTest)
    {
        std::string str = "rect";
        EXPECT_EQ(CommandParser::GetInstance().GetScreenShape(), str);
    }

    TEST_F(CommandParserTest, GetProjectModelTest)
    {
        std::string str = "Stage";
        EXPECT_EQ(CommandParser::GetInstance().GetProjectModel(), str);
    }

    TEST_F(CommandParserTest, GetProjectModelEnumValueTest)
    {
        int value = 1;
        EXPECT_EQ(CommandParser::GetInstance().GetProjectModelEnumValue(), value);
    }

    TEST_F(CommandParserTest, GetProjectModelEnumNameTest)
    {
        int index = 0;
        EXPECT_EQ(CommandParser::GetInstance().GetProjectModelEnumName(index), "FA");
        index = 1;
        EXPECT_EQ(CommandParser::GetInstance().GetProjectModelEnumName(index), "Stage");
        index = -1;
        EXPECT_EQ(CommandParser::GetInstance().GetProjectModelEnumName(index), "FA");
        index = 2;
        EXPECT_EQ(CommandParser::GetInstance().GetProjectModelEnumName(index), "FA");
    }

    TEST_F(CommandParserTest, GetPagesTest)
    {
        std::string str = "main_pages";
        EXPECT_EQ(CommandParser::GetInstance().GetPages(), str);
    }

    TEST_F(CommandParserTest, GetContainerSdkPathTest)
    {
        EXPECT_EQ(CommandParser::GetInstance().GetContainerSdkPath(), currDir);
    }

    TEST_F(CommandParserTest, CheckParamInvalidityTest)
    {
        std::string param = "0";
        EXPECT_FALSE(CommandParser::GetInstance().CheckParamInvalidity(param, true));
        EXPECT_FALSE(CommandParser::GetInstance().CheckParamInvalidity(param, false));
        param = "110";
        EXPECT_FALSE(CommandParser::GetInstance().CheckParamInvalidity(param, true));
        EXPECT_FALSE(CommandParser::GetInstance().CheckParamInvalidity(param, false));
        param = "01";
        EXPECT_TRUE(CommandParser::GetInstance().CheckParamInvalidity(param, true));
        EXPECT_FALSE(CommandParser::GetInstance().CheckParamInvalidity(param, false));
        param = "ab01";
        EXPECT_TRUE(CommandParser::GetInstance().CheckParamInvalidity(param, true));
        EXPECT_FALSE(CommandParser::GetInstance().CheckParamInvalidity(param, false));
        param = "0.34";
        EXPECT_FALSE(CommandParser::GetInstance().CheckParamInvalidity(param, true));
        EXPECT_FALSE(CommandParser::GetInstance().CheckParamInvalidity(param, false));
        param = "10.34";
        EXPECT_FALSE(CommandParser::GetInstance().CheckParamInvalidity(param, true));
        EXPECT_FALSE(CommandParser::GetInstance().CheckParamInvalidity(param, false));
    }

    TEST_F(CommandParserTest, IsComponentModeTest)
    {
        EXPECT_TRUE(CommandParser::GetInstance().IsComponentMode());
    }

    TEST_F(CommandParserTest, GetAbilityPathTest)
    {
        std::string str = "ets/entryability/EntryAbility.abc";
        EXPECT_EQ(CommandParser::GetInstance().GetAbilityPath(), str);
    }

    TEST_F(CommandParserTest, GetAbilityNameTest)
    {
        std::string str = "EntryAbility";
        EXPECT_EQ(CommandParser::GetInstance().GetAbilityName(), str);
    }

    TEST_F(CommandParserTest, IsStaticCardTest)
    {
        EXPECT_TRUE(CommandParser::GetInstance().IsStaticCard());
    }

    TEST_F(CommandParserTest, IsMainArgLengthInvalidTest)
    {
        std::string validStr = "1111111";
        EXPECT_FALSE(CommandParser::GetInstance().IsMainArgLengthInvalid(validStr.c_str()));
        std::string invalidStr = R"(11111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111
            11111111111111111111111111111111111111111111111111111111111111111111111111111111)";
        EXPECT_TRUE(CommandParser::GetInstance().IsMainArgLengthInvalid(invalidStr.c_str()));
    }

    TEST_F(CommandParserTest, IsFoldableTest)
    {
        EXPECT_TRUE(CommandParser::GetInstance().IsFoldable());
    }

    TEST_F(CommandParserTest, GetFoldStatusTest)
    {
        std::string str = "half_fold";
        EXPECT_EQ(CommandParser::GetInstance().GetFoldStatus(), str);
    }

    TEST_F(CommandParserTest, GetFoldResolutionWidthTest)
    {
        int32_t value = 1080;
        EXPECT_EQ(CommandParser::GetInstance().GetFoldResolutionWidth(), value);
    }

    TEST_F(CommandParserTest, GetFoldResolutionHeightTest)
    {
        int32_t value = 2504;
        EXPECT_EQ(CommandParser::GetInstance().GetFoldResolutionHeight(), value);
    }

    TEST_F(CommandParserTest, GetLoaderJsonPathTest)
    {
        EXPECT_EQ(CommandParser::GetInstance().GetLoaderJsonPath(), currFile);
    }

}