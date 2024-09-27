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
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include "gtest/gtest.h"
#define private public
#define protected public
#include "CommandLineInterface.h"
#include "FileSystem.h"
#include "JsAppImpl.h"
#include "TraceTool.h"
#include "CommandParser.h"

namespace {
    class JsAppImplTest : public ::testing::Test {
    public:
        JsAppImplTest() {}
        ~JsAppImplTest() {}
    protected:
        static void CopyFileToDirectory(const std::filesystem::path& sourceFile,
            const std::filesystem::path& destDirectory)
        {
            // 确保目标目录存在
            if (!std::filesystem::exists(destDirectory)) {
                std::filesystem::create_directories(destDirectory);
            }
            // 生成目标文件路径
            std::filesystem::path destFile = destDirectory / sourceFile.filename();
            // 复制文件
            std::filesystem::copy_file(sourceFile, destFile,
                std::filesystem::copy_options::overwrite_existing);
            std::cout << "File copied successfully to " << destFile << std::endl;
        }

        static void SetUpTestCase()
        {
            CommandLineInterface::GetInstance().Init("pipeName");
            TraceTool::GetInstance().InitPipe();
            testConfigPath = FileSystem::GetApplicationPath() + FileSystem::separator + ".." +
                FileSystem::separator + "config" + FileSystem::separator;
            std::string srcBrkPath = FileSystem::GetApplicationPath() + FileSystem::separator +
                "foundation/arkui/ui_lite/tools/qt/simulator/font/line_cj.brk";
            std::string srcFontPath = FileSystem::GetApplicationPath() + FileSystem::separator +
                "foundation/arkui/ui_lite/tools/qt/simulator/font/SourceHanSansSC-Regular.otf";
            CopyFileToDirectory(srcBrkPath, testConfigPath);
            CopyFileToDirectory(srcFontPath, testConfigPath);
        }

        static std::string testConfigPath;
    };
    std::string JsAppImplTest::testConfigPath = "";

    // 测试拷贝构造函数是否被删除
    TEST_F(JsAppImplTest, CopyConstructorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_constructible<JsApp>::value == false);
    }

    // 测试赋值运算符是否被删除
    TEST_F(JsAppImplTest, AssignmentOperatorDeletedTest)
    {
        EXPECT_TRUE(std::is_copy_assignable<JsApp>::value == false);
    }

    TEST_F(JsAppImplTest, StartTest)
    {
        JsAppImpl::GetInstance().isFinished = true;
        JsAppImpl::GetInstance().Start();
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10 ms
        JsAppImpl::GetInstance().isInterrupt = true;
        EXPECT_FALSE(JsAppImpl::GetInstance().isFinished);
    }

    TEST_F(JsAppImplTest, InterruptTest)
    {
        JsAppImpl::GetInstance().jsAbility = std::make_unique<OHOS::ACELite::JSAbility>();
        JsAppImpl::GetInstance().Interrupt();
        EXPECT_TRUE(JsAppImpl::GetInstance().isFinished);
        EXPECT_TRUE(JsAppImpl::GetInstance().isInterrupt);
    }

    TEST_F(JsAppImplTest, InitJsAppTest)
    {
        JsAppImpl::GetInstance().isFinished = true;
        CommandParser::GetInstance().argsMap.clear();
        CommandParser::GetInstance().argsMap["-s"] = { "" };
        CommandParser::GetInstance().argsMap["-lws"] = { "" };
        CommandParser::GetInstance().argsMap["-n"] = { "" };
        CommandParser::GetInstance().argsMap["-hs"] = { "" };
        CommandParser::GetInstance().argsMap["-d"] = { "" };
        CommandParser::GetInstance().argsMap["-p"] = { "" };
        CommandParser::GetInstance().argsMap["-j"] = { "" };
        CommandParser::GetInstance().argsMap["-url"] = { "" };
        JsAppImpl::GetInstance().InitJsApp();
        CommandParser::GetInstance().argsMap.clear();
        JsAppImpl::GetInstance().isInterrupt = true;
        EXPECT_FALSE(JsAppImpl::GetInstance().isFinished);
    }

    TEST_F(JsAppImplTest, StartJsAppTest1)
    {
        // jsAbility not nullptr
        JsAppImpl::GetInstance().isFinished = true;
        JsAppImpl::GetInstance().jsAbility = std::make_unique<OHOS::ACELite::JSAbility>();
        JsAppImpl::GetInstance().StartJsApp();
        EXPECT_TRUE(JsAppImpl::GetInstance().isFinished);
    }

    TEST_F(JsAppImplTest, StartJsAppTest2)
    {
        JsAppImpl::GetInstance().jsAbility = nullptr;
        JsAppImpl::GetInstance().isFinished = true;
        // urlPath empty
        JsAppImpl::GetInstance().isDebug = true;
        JsAppImpl::GetInstance().debugServerPort = 9999;
        JsAppImpl::GetInstance().urlPath = "";
        JsAppImpl::GetInstance().StartJsApp();
        EXPECT_TRUE(JsAppImpl::GetInstance().isFinished);
    }

    TEST_F(JsAppImplTest, StartJsAppTest3)
    {
        JsAppImpl::GetInstance().jsAbility = nullptr;
        JsAppImpl::GetInstance().isFinished = true;
        // urlPath not empty
        JsAppImpl::GetInstance().isDebug = true;
        JsAppImpl::GetInstance().debugServerPort = 9999;
        JsAppImpl::GetInstance().urlPath = "pages/Index";
        JsAppImpl::GetInstance().StartJsApp();
        EXPECT_FALSE(JsAppImpl::GetInstance().isFinished);
    }
}