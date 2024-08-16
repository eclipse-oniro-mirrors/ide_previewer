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
#include <iostream>
#include <fstream>
#include <filesystem>
#include "cJSON.h"
#include <gtest/gtest.h>
#include "secodeFuzz.h"
#include "common.h"
#include "ChangeJsonUtil.h"
#include "StageContext.h"
using namespace fuzztest;

namespace {
std::string g_mockConfigJson = R"({
    "libentry.so": {
        "source": "src/mock/libentry.mock.ets"
    }
})";

TEST(ParseMockJsonFileFuzzTest, test_json)
{
    std::cout << "--> ParseMockJsonFileFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"ParseMockJsonFileFuzzTest", 0)
    {
        uint64_t index = 0;
        // 变化数据
        cJSON* jsonArgs = cJSON_Parse(g_mockConfigJson.c_str());
        ChangeJsonUtil::ModifyObject(jsonArgs, index);
        // 保存到文件
        std::string filePath = "mock-config.json";
        ChangeJsonUtil::WriteFile(filePath, jsonArgs);
        cJSON_Delete(jsonArgs);
        // 执行接口
        OHOS::Ide::StageContext::GetInstance().ParseMockJsonFile("aaa");
        OHOS::Ide::StageContext::GetInstance().ParseMockJsonFile(filePath);
        ChangeJsonUtil::WriteFile(filePath, "aaa");
        OHOS::Ide::StageContext::GetInstance().ParseMockJsonFile(filePath);
    }
    DT_FUZZ_END()
    printf("end ---- ParseMockJsonFileFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test ParseMockJsonFileFuzzTest is not ok\r\n");
    } else {
        printf("test ParseMockJsonFileFuzzTest is ok\r\n");
    }
    std::cout << "--> ParseMockJsonFileFuzzTest for common end <--" << std::endl;
}
}