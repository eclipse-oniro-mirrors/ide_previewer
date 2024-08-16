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
#include <gtest/gtest.h>
#include "secodeFuzz.h"
#include "common.h"
#include "CommandParse.h"
#include "CommandLineInterface.h"
#define private public
#include "CommandParser.h"
using namespace std;
using namespace fuzztest;

namespace {
std::map<std::string, std::string> commonDataMap = {
    {"MousePress", R"({"x":365,"y":1076,"duration":""})"},
    {"MouseRelease", R"({"x":365,"y":1071})"},
    {"MouseMove", R"({"x":365,"y":1071,"duration":79})"},
    {"Language", R"({"Language":"zh_CN"})"},
    {"SupportedLanguages", ""},
    {"exit", ""},
    {"Resolution", ""},
    {"CurrentRouter", ""},
    {"LoadContent", ""},
    {"UnsupportedCommond", ""},
    {"KeyPress", R"({"isInputMethod":true,"codePoint":33})"},
};

TEST(CommonCommandParseFuzzTest, test_command)
{
    std::cout << "--> CommonCommandParseFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"CommonCommandParseFuzzTest", 0)
    {
        CommandParse parse;
        CommandParser::GetInstance().deviceType = "common";
        parse.CreateAndExecuteCommand(commonDataMap);
    }
    DT_FUZZ_END()
    printf("end ---- CommonCommandParseFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test CommonCommandParseFuzzTest is not ok\r\n");
    } else {
        printf("test CommonCommandParseFuzzTest is ok\r\n");
    }
    std::cout << "--> CommonCommandParseFuzzTest for common end <--" << std::endl;
}
}