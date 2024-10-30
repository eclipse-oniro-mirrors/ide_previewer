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
        "-sid abcDef123"
        "-h";

    TEST_F(CommandParserTest, IsSetTest)
    {
        CommandParser::GetInstance().argsMap.clear();
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsSet("arp"));
        EXPECT_FALSE(CommandParser::GetInstance().IsSet("abc"));
    }

    TEST_F(CommandParserTest, IsCommandValidTest2)
    {
        CommandParser::GetInstance().argsMap.clear();
        EXPECT_FALSE(CommandParser::GetInstance().ProcessCommand(invalidParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
    }

    TEST_F(CommandParserTest, IsCommandValidTest_VErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        invalidParamVec.push_back("-v");
        EXPECT_FALSE(CommandParser::GetInstance().ProcessCommand(invalidParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
    }

    TEST_F(CommandParserTest, IsCommandValidTest_PErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-p");
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "new_value";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "655350";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "8888";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_JErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-j");
        if (it != validParamVec.end()) {
            *it = "-jjjjj";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end()) {
            *it = "-j";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_NErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-n");
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "~!@#$%^&";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            std::string str(257, 'a');
            *std::next(it) = str;
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "entry";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_CrOrErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        // no -cr param
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-cr");
        if (it != validParamVec.end()) {
            *it = "~!@#$%^&";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end()) {
            *it = "-cr";
        }
        // -cr value invalid
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "aaaa";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        // params value invalid
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "4000";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "1080";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_HsErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-hs");
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "~!@#$%^&";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "524289";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "100000";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_HfErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-hf");
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "~!@#$%^&";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "true";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_ShapeErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-shape");
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "~!@#$%^&";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "rect";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_DeviceErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-device");
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "~!@#$%^&";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "phone";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_UrlErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-url");
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "pages/BigImg_1M";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_FErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-f");
        if (it != validParamVec.end()) {
            *it = "ffff";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-f";
            *std::next(it) = "pages/BigImg_1M";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = currFile;
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_ArpErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-arp");
        if (it != validParamVec.end()) {
            *it = "aaarp";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-arp";
            *std::next(it) = "pages/BigImg_1M";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = currDir;
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_PmErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-pm");
        if (it != validParamVec.end()) {
            *it = "aaapm";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-pm";
            *std::next(it) = "aaa";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "Stage";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_PagesErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-pages");
        if (it != validParamVec.end()) {
            *it = "aaapages";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-pages";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "main_pages";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_RefreshErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-refresh");
        if (it != validParamVec.end()) {
            *it = "aaarefresh";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-refresh";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "region";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_CardErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-card");
        if (it != validParamVec.end()) {
            *it = "aaacard";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-card";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "true";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_ProjectIdErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-projectID");
        if (it != validParamVec.end()) {
            *it = "aaaprojectID";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-projectID";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "138968279";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_CmErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-cm");
        if (it != validParamVec.end()) {
            *it = "aaacm";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-cm";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "light";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_AvErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-av");
        if (it != validParamVec.end()) {
            *it = "aaaav";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-av";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "ACE_2_0";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_OErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-o");
        if (it != validParamVec.end()) {
            *it = "aaao";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-o";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "portrait";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_LwsErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-lws");
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "65536";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "40003";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_SmErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-sm");
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "static";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_HspErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-hsp");
        if (it != validParamVec.end()) {
            *it = "aaahsp";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-hsp";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = currDir;
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_CpmErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-cpm");
        if (it != validParamVec.end()) {
            *it = "aaacpm";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-cpm";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "true";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_AbpErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        // lite device
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-device");
        if (it != validParamVec.end()) {
            *std::next(it) = "liteWearable";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end()) {
            *std::next(it) = "phone";
        }
        // no -d
        it = std::find(validParamVec.begin(), validParamVec.end(), "-d");
        if (it != validParamVec.end()) {
            *it = "aaad";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end()) {
            *it = "-d";
        }
        // no -abp
        CommandParser::GetInstance().argsMap.clear();
        it = std::find(validParamVec.begin(), validParamVec.end(), "-abp");
        if (it != validParamVec.end()) {
            *it = "aaaabp";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        // -abp value empty
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-abp";
            *std::next(it) = "";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "ets/entryability/EntryAbility.abc";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_AbnErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        // no -d
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-d");
        if (it != validParamVec.end()) {
            *it = "aaad";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end()) {
            *it = "-d";
        }
        // lite device
        it = std::find(validParamVec.begin(), validParamVec.end(), "-device");
        if (it != validParamVec.end()) {
            *std::next(it) = "liteWearable";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end()) {
            *std::next(it) = "phone";
        }
        // no -abn
        CommandParser::GetInstance().argsMap.clear();
        it = std::find(validParamVec.begin(), validParamVec.end(), "-abn");
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "aaaabn";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        // -abn value empty
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-abn";
            *std::next(it) = "";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "EntryAbility";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_StaticCardErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-staticCard");
        if (it != validParamVec.end()) {
            *it = "aaastaticCard";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-staticCard";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "true";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_FoldableErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-foldable");
        if (it != validParamVec.end()) {
            *it = "aaafoldable";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-foldable";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "true";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_FoldStatusErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        // no -foldable
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-foldable");
        if (it != validParamVec.end()) {
            *it = "aaafoldable";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        // param -foldable value is false
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-foldable";
            *std::next(it) = "false";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "true";
        }
        // param foldStatus value invalid
        CommandParser::GetInstance().argsMap.clear();
        it = std::find(validParamVec.begin(), validParamVec.end(), "-foldStatus");
        if (it != validParamVec.end()) {
            *it = "aaafoldStatus";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end()) {
            *it = "-foldStatus";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_FrErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        // no -foldable
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-foldable");
        if (it != validParamVec.end()) {
            *it = "aaafoldable";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        // param -foldable value is false
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-foldable";
            *std::next(it) = "false";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "true";
        }
        // no param -fr
        CommandParser::GetInstance().argsMap.clear();
        it = std::find(validParamVec.begin(), validParamVec.end(), "-fr");
        if (it != validParamVec.end()) {
            *it = "aaafr";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end()) {
            *it = "-fr";
        }
        // param -fr value is invalid
        it = std::find(validParamVec.begin(), validParamVec.end(), "-fr");
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "aaaa";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "1080";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_LjPathErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-ljPath");
        if (it != validParamVec.end()) {
            *it = "aaaljPath";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-ljPath";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = currFile;
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_LErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-l");
        if (it != validParamVec.end()) {
            *it = "aaal";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-l";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "zh_CN";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_TsErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-ts");
        if (it != validParamVec.end()) {
            *it = "aaats";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-ts";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "trace_70259_commandPipe";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_SErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-s");
        if (it != validParamVec.end()) {
            *it = "aaas";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-s";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "componentpreviewinstance_1712054594321_1";
        }
    }

    TEST_F(CommandParserTest, IsCommandValidTest_SdErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-sd");
        if (it != validParamVec.end()) {
            *it = "aaasd";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-sd";
            *std::next(it) = "~!@#$%^";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "480";
        }
    }


    TEST_F(CommandParserTest, IsCommandValidTest1)
    {
        CommandParser::GetInstance().argsMap.clear();
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
    }

    TEST_F(CommandParserTest, ValueTest)
    {
        std::string str1 = "abcd";
        std::string str2 = "rect";
        std::string str3 = CommandParser::GetInstance().Value("shape");
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
        std::string helpStr = CommandParser::GetInstance().regsHelpMap[key];
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
        int desiredLength = 1184;
        char initialChar = '1';
        std::string invalidStr(desiredLength, initialChar);
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

    TEST_F(CommandParserTest, GetCommandInfoTest)
    {
        CommandInfo info;
        CommandParser::GetInstance().GetCommandInfo(info);
        EXPECT_EQ(info.deviceType, "phone");
        EXPECT_TRUE(info.isComponentMode);
    }

    TEST_F(CommandParserTest, GetFoldInfoTest)
    {
        FoldInfo info;
        CommandParser::GetInstance().GetFoldInfo(info);
        EXPECT_TRUE(info.foldable);
        EXPECT_EQ(info.foldStatus, "half_fold");
    }

    TEST_F(CommandParserTest, ParseArgsTest)
    {
        int argc = 5;
        char* argv[5]; // 5 is array length
        // param value length is out of range
        int desiredLength = 1184;
        char initialChar = '1';
        std::string invalidStr(desiredLength, initialChar);
        argv[1] = "-p";
        argv[2] = const_cast<char*>(invalidStr.c_str());
        int ret = CommandParser::GetInstance().ParseArgs(argc, argv);
        EXPECT_EQ(ret, 11); // 11 is expect return value
        // param value invalid
        argv[2] = "999999999";
        ret = CommandParser::GetInstance().ParseArgs(argc, argv);
        EXPECT_EQ(ret, 11); // 11 is expect return value
        argv[2] = "9999";
        ret = CommandParser::GetInstance().ParseArgs(argc, argv);
        EXPECT_EQ(ret, -1); // -1 is expect return valu
        // invalid param
        argv[3] = "-v";
        ret = CommandParser::GetInstance().ParseArgs(argc, argv);
        EXPECT_EQ(ret, 0); // 0 is expect return value
    }

    TEST_F(CommandParserTest, IsCommandValidTest_SidErr)
    {
        CommandParser::GetInstance().argsMap.clear();
        auto it = std::find(validParamVec.begin(), validParamVec.end(), "-sid");
        if (it != validParamVec.end()) {
            *it = "aaasid";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *it = "-sid";
            *std::next(it) = "abcdefg123";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_FALSE(CommandParser::GetInstance().IsCommandValid());
        if (it != validParamVec.end() && std::next(it) != validParamVec.end()) {
            *std::next(it) = "abcDef123";
        }
        EXPECT_TRUE(CommandParser::GetInstance().ProcessCommand(validParamVec));
        EXPECT_TRUE(CommandParser::GetInstance().IsCommandValid());
    }
}