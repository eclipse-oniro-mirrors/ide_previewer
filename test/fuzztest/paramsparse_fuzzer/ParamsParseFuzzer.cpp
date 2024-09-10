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

#include "ParamsParseFuzzer.h"
#include "common.h"
#include "secodeFuzz.h"
#include "CommandParser.h"

using namespace fuzztest;

const int DEFAULT_LENGTH = 1000;

void ParamsParse::CallParamsParseFunc(const std::vector<std::string>& args)
{
    CommandParser& parser = CommandParser::GetInstance();
    parser.ProcessCommand(args);
    parser.IsCommandValid();
}

void ParamsParse::ParamsParseFuzzTest()
{
    // 要求单测试用例最低运行3千万次或者8小时以上
    printf("start ---- ParamsParseFuzzTest\r\n");
    DT_FUZZ_START(0, TEST_TIMES, (char*)"ParamsParseFuzzTest", 0)
    {
        std::vector<std::string> args;
        SetTestArgs(args);
        CallParamsParseFunc(args);
    }
    DT_FUZZ_END()
    printf("end ---- ParamsParseFuzzTest\r\n");

    if (DT_GetIsPass() == 0) {
        printf("ParamsParseFuzzTest is not ok\r\n");
    } else {
        printf("ParamsParseFuzzTest is ok\r\n");
    }
}

void ParamsParse::SetTestArgs(std::vector<std::string>& args)
{
    int index = -1;
    for (const Param& param : paramList) {
        args.push_back(param.name);
        for (const std::string& value : param.values) {
            index++;
            std::string str = std::string(value);
            args.push_back(DT_SetGetString(&g_Element[index], str.size() + 1, DEFAULT_LENGTH, (char*)str.c_str()));
        }
    }
}

namespace {
TEST_F(ParamsParseFuzzTest, test_params)
{
    parse.ParamsParseFuzzTest();
}
}