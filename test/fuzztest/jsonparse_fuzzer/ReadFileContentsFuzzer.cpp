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
#include <fstream>
#include <iostream>
#include <filesystem>
#include <gtest/gtest.h>
#include "cJSON.h"
#include "secodeFuzz.h"
#include "common.h"
#include "StageContext.h"
#include "ChangeJsonUtil.h"
#include "MockFile.h"
using namespace fuzztest;
using namespace testmock;

namespace {
std::string g_moduleJson = R"({
    "modulePathMap" : {
        "entry" : "C:\\MyApp1\\entry",
        "library" : "C:\\MyApp1\\library"
    },
    "compileMode" : "esmodule",
    "projectRootPath" : "C:\\MyApp1",
    "nodeModulesPath" : "C:\\MyApp1\\entry\\.preview\\default\\intermediates\\loader_out\\default\\node_modules",
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
    "anBuildOutPut" : "C:\\MyApp1\\entry\\.preview\\default\\intermediates\\loader_out\\default\\an\\arm64-v8a",
    "anBuildMode" : "type",
    "buildConfigPath" : ".preview\\config\\buildConfig.json"
})";

TEST(ReadFileContentsFuzzTest, test_json)
{
    std::cout << "--> ReadFileContentsFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"ReadFileContentsFuzzTest", 0)
    {
        uint64_t index = 0;
        // 变化数据
        cJSON* jsonArgs = cJSON_Parse(g_moduleJson.c_str());
        ChangeJsonUtil::ModifyObject(jsonArgs, index);
        // 保存到文件
        std::string filePath = "testFile.json";
        ChangeJsonUtil::WriteFile(filePath, jsonArgs);
        cJSON_Delete(jsonArgs);
        // 执行接口
        OHOS::Ide::StageContext::GetInstance().ReadFileContents("aaa");
        MockFile::SimulateFileLock(filePath);
        OHOS::Ide::StageContext::GetInstance().ReadFileContents(filePath);
        MockFile::ReleaseFileLock(filePath);
        OHOS::Ide::StageContext::GetInstance().ReadFileContents(filePath);
    }
    DT_FUZZ_END()
    printf("end ---- ReadFileContentsFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test ReadFileContentsFuzzTest is not ok\r\n");
    } else {
        printf("test ReadFileContentsFuzzTest is ok\r\n");
    }
    std::cout << "--> ReadFileContentsFuzzTest for common end <--" << std::endl;
}
}