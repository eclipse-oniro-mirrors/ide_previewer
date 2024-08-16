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
#include <map>
#include <iostream>
#include <filesystem>
#include "cJSON.h"
#include <gtest/gtest.h>
#include "secodeFuzz.h"
#include "StageContext.h"
#include "common.h"
#include "ChangeJsonUtil.h"
using namespace fuzztest;

namespace {
std::string g_moduleJson1 = R"({
    "modulePathMap" : {
        "entry" : "C:\\MyApp\\entry",
        "library" : "C:\\MyApp\\library"
    },
    "compileMode" : "esmodule",
    "projectRootPath" : "C:\\MyApp",
    "nodeModulesPath" : "C:\\MyApp\\entry\\.preview\\default\\intermediates\\loader_out\\default\\node_modules",
    "moduleName" : "entry",
    "hspNameOhmMap" : {
        "library" : "@bundle:com.example.myapplication/library/Index"
    },
    "harNameOhmMap" : {
        "library" : "@bundle:com.example.myapplication/library/Index"
    },
    "packageManagerType" : "ohpm",
    "compileEntry" : [],
    "dynamicImportLibInfo" : {},
    "anBuildOutPut" : "C:\\MyApp\\entry\\.preview\\default\\intermediates\\loader_out\\default\\an\\arm64-v8a",
    "anBuildMode" : "type",
    "buildConfigPath" : ".preview\\config\\buildConfig.json"
})";

std::string g_moduleJson2 = R"({
    "projectRootPath": "C:\\MyApp"
})";

TEST(GetModulePathMapFuzzTest, test_json)
{
    std::cout << "--> GetModulePathMapFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"GetModulePathMapFuzzTest", 0)
    {
        uint64_t index = 0;
        // 变化数据
        cJSON* jsonArgs = cJSON_Parse(g_moduleJson1.c_str());
        ChangeJsonUtil::ModifyObject(jsonArgs, index);
        // 保存到文件
        std::string filePath = "loader.json";
        ChangeJsonUtil::WriteFile(filePath, jsonArgs);
        cJSON_Delete(jsonArgs);
        // 执行接口
        OHOS::Ide::StageContext::GetInstance().SetLoaderJsonPath("");
        OHOS::Ide::StageContext::GetInstance().GetModulePathMapFromLoaderJson();
        OHOS::Ide::StageContext::GetInstance().SetLoaderJsonPath(filePath);
        OHOS::Ide::StageContext::GetInstance().GetModulePathMapFromLoaderJson();
        ChangeJsonUtil::WriteFile(filePath, g_moduleJson2);
        OHOS::Ide::StageContext::GetInstance().GetModulePathMapFromLoaderJson();
        ChangeJsonUtil::WriteFile(filePath, "aaa");
        OHOS::Ide::StageContext::GetInstance().GetModulePathMapFromLoaderJson();
    }
    DT_FUZZ_END()
    printf("end ---- GetModulePathMapFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test GetModulePathMapFuzzTest is not ok\r\n");
    } else {
        printf("test GetModulePathMapFuzzTest is ok\r\n");
    }
    std::cout << "--> GetModulePathMapFuzzTest for common end <--" << std::endl;
}
}