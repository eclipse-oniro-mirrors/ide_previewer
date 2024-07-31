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
#include "gtest/gtest.h"
#define private public
#include "CommandLineFactory.h"
#include "CommandParser.h"
using namespace std;

namespace {
    TEST(CommandLineFactoryTest, DefaultConstructorBehaviorTest)
    {
        CommandLineFactory factory;
        factory.InitCommandMap();
        EXPECT_TRUE(factory.typeMap.size() > 0);
    }

    TEST(CommandLineFactoryTest, InitCommandMapTest)
    {
        string deviceType = "phone";
        CommandParser::GetInstance().deviceType = deviceType;
        CommandLineFactory::InitCommandMap();
        EXPECT_TRUE(CommandLineFactory::typeMap.size() > 0);
    }

    TEST(CommandLineFactoryTest, CreateCommandLineTest)
    {
        std::string commandName = "ColorMode";
        std::string jsonStr = R"({"ColorMode":"dark"})";
        Json2::Value jsonData = JsonReader::ParseJsonData2(jsonStr);
        std::unique_ptr<LocalSocket> socket = std::make_unique<LocalSocket>();
        
        CommandLine::CommandType commandType = CommandLine::CommandType::SET;
        std::string commandNameNull = "ColorMode1";
        std::unique_ptr<CommandLine> commandLineNull =
            CommandLineFactory::CreateCommandLine(commandNameNull, commandType, jsonData, *socket);
        EXPECT_TRUE(commandLineNull == nullptr);

        std::unique_ptr<CommandLine> commandLine =
            CommandLineFactory::CreateCommandLine(commandName, commandType, jsonData, *socket);
        EXPECT_FALSE(commandLine == nullptr);
    }
}
