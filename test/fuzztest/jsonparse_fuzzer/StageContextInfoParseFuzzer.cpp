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

std::string g_mockConfigJson = R"({
    "libentry.so": {
        "source": "src/mock/libentry.mock.ets"
    }
})";

std::string g_buildConfigJson1 = R"({
    "deviceType": "phone,tablet,2in1",
    "aceModuleBuild": "C:\\MyApp\\entry\\.preview\\default\\intermediates\\assets\\default\\ets"
})";

std::string g_buildConfigJson2 = R"({
    "deviceType": "phone,tablet,2in1",
    "checkEntry": "true"
})";

TEST(LoaderInfoParseFuzzTest, test_json)
{
    std::cout << "--> LoaderInfoParseFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"LoaderInfoParseFuzzTest", 0)
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
    printf("end ---- LoaderInfoParseFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test LoaderInfoParseFuzzTest is not ok\r\n");
    } else {
        printf("test LoaderInfoParseFuzzTest is ok\r\n");
    }
    std::cout << "--> LoaderInfoParseFuzzTest for common end <--" << std::endl;
}

TEST(MockConfigParseFuzzTest, test_json)
{
    std::cout << "--> MockConfigParseFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"MockConfigParseFuzzTest", 0)
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
    printf("end ---- MockConfigParseFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test MockConfigParseFuzzTest is not ok\r\n");
    } else {
        printf("test MockConfigParseFuzzTest is ok\r\n");
    }
    std::cout << "--> MockConfigParseFuzzTest for common end <--" << std::endl;
}

TEST(BuildConfigParseFuzzTest, test_json)
{
    std::cout << "--> BuildConfigParseFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"BuildConfigParseFuzzTest", 0)
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
    printf("end ---- BuildConfigParseFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test BuildConfigParseFuzzTest is not ok\r\n");
    } else {
        printf("test BuildConfigParseFuzzTest is ok\r\n");
    }
    std::cout << "--> BuildConfigParseFuzzTest for common end <--" << std::endl;
}

TEST(HspInfoParseFuzzTest, test_json)
{
    std::cout << "--> HspInfoParseFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"HspInfoParseFuzzTest", 0)
    {
        // 变化数据
        const int defaultLength = 1000;
        const std::string defaultString = "aaaa";
        std::string strVal = DT_SetGetString(&g_Element[0], defaultString.size() + 1, defaultLength,
            (char*)defaultString.c_str());
        std::string newFileName = MockFile::CreateHspFile("testHspFile", strVal);
        // 执行接口
        OHOS::Ide::StageContext::GetInstance().GetModuleBufferFromHsp("aaa", "bbb");
        OHOS::Ide::StageContext::GetInstance().GetModuleBufferFromHsp(newFileName, "bbb.txt");
        std::vector<uint8_t>* buf =
            OHOS::Ide::StageContext::GetInstance().GetModuleBufferFromHsp(newFileName, "ets/modules.abc");
        if (buf) {
            delete buf;
        }
    }
    DT_FUZZ_END()
    printf("end ---- HspInfoParseFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test HspInfoParseFuzzTest is not ok\r\n");
    } else {
        printf("test HspInfoParseFuzzTest is ok\r\n");
    }
    std::cout << "--> HspInfoParseFuzzTest for common end <--" << std::endl;
}

TEST(FileBufferFuzzTest, test_json)
{
    std::cout << "--> FileBufferFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"FileBufferFuzzTest", 0)
    {
        uint64_t index = 0;
        // 变化数据
        cJSON* jsonArgs = cJSON_Parse(g_moduleJson1.c_str());
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
    printf("end ---- FileBufferFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test FileBufferFuzzTest is not ok\r\n");
    } else {
        printf("test FileBufferFuzzTest is ok\r\n");
    }
    std::cout << "--> FileBufferFuzzTest for common end <--" << std::endl;
}
}