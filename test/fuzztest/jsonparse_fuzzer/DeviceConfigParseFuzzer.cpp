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
#include "cJSON.h"
#include "secodeFuzz.h"
#include "common.h"
#include "CommandLineInterface.h"
#include "ChangeJsonUtil.h"
#define private public
#include "CommandParser.h"
using namespace std;
using namespace fuzztest;

namespace {
std::string g_deviceConfigJson = R"({
    "setting" : {
        "1.0.1" : {
            "Language" : {
                "args" : {
                    "Language" : "zh-CN"
                }
            }
        }
    },
    "frontend" : {
        "1.0.0" : {
            "Resolution" : {
                "args" : {
                    "Resolution" : "360*780"
                }
            },
            "DeviceType" : {
                "args" : {
                    "DeviceType" : "phone"
                }
            }
        }
    }
})";

TEST(DeviceConfigParseFuzzTest, test_json)
{
    std::cout << "--> DeviceConfigParseFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"DeviceConfigParseFuzzTest", 0)
    {
        CommandLineInterface::GetInstance().Init("pipeName");
        CommandParser::GetInstance().deviceType = "phone";
        uint64_t index = 0;
        // 变化数据
        cJSON* jsonArgs = cJSON_Parse(g_deviceConfigJson.c_str());
        ChangeJsonUtil::ModifyObject(jsonArgs, index);
        // 保存到文件
        std::string filePath = "phoneSettingConfig_Phone.json";
        ChangeJsonUtil::WriteFile(filePath, jsonArgs);
        cJSON_Delete(jsonArgs);
        // 执行接口
        CommandLineInterface::GetInstance().ReadAndApplyConfig(filePath);
        CommandLineInterface::GetInstance().ReadAndApplyConfig("");
    }
    DT_FUZZ_END()
    printf("end ---- DeviceConfigParseFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test DeviceConfigParseFuzzTest is not ok\r\n");
    } else {
        printf("test DeviceConfigParseFuzzTest is ok\r\n");
    }
    std::cout << "--> DeviceConfigParseFuzzTest for common end <--" << std::endl;
}
}