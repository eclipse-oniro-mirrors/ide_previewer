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
#include <gtest/gtest.h>
#include "secodeFuzz.h"
#include "cJSON.h"
#include "common.h"
#include "StageContext.h"
#include "ChangeJsonUtil.h"
using namespace fuzztest;

namespace {
std::string g_buildConfigJson1 = R"({
    "deviceType": "phone,tablet,2in1",
    "aceModuleBuild": "C:\\MyApp\\entry\\.preview\\default\\intermediates\\assets\\default\\ets"
})";

std::string g_buildConfigJson2 = R"({
    "deviceType": "phone,tablet,2in1",
    "checkEntry": "true"
})";

TEST(GetHspAceModuleBuildFuzzTest, test_json)
{
    std::cout << "--> GetHspAceModuleBuildFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"GetHspAceModuleBuildFuzzTest", 0)
    {
        std::string filePath = "buildConfig.json";
        OHOS::Ide::StageContext::GetInstance().GetHspAceModuleBuild("aaa");
        ChangeJsonUtil::WriteFile(filePath, "aaa");
        OHOS::Ide::StageContext::GetInstance().GetHspAceModuleBuild(filePath);
        ChangeJsonUtil::WriteFile(filePath, g_buildConfigJson2);
        OHOS::Ide::StageContext::GetInstance().GetHspAceModuleBuild(filePath);
        uint64_t index = 0;
        // 变化数据
        cJSON* jsonArgs = cJSON_Parse(g_buildConfigJson1.c_str());
        ChangeJsonUtil::ModifyObject(jsonArgs, index);
        // 保存到文件
        ChangeJsonUtil::WriteFile(filePath, jsonArgs);
        cJSON_Delete(jsonArgs);
        // 执行接口
        OHOS::Ide::StageContext::GetInstance().GetHspAceModuleBuild(filePath);
    }
    DT_FUZZ_END()
    printf("end ---- GetHspAceModuleBuildFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test GetHspAceModuleBuildFuzzTest is not ok\r\n");
    } else {
        printf("test GetHspAceModuleBuildFuzzTest is ok\r\n");
    }
    std::cout << "--> GetHspAceModuleBuildFuzzTest for common end <--" << std::endl;
}
}