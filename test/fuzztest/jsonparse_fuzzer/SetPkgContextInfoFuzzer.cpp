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

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "cJSON.h"
#include <gtest/gtest.h>
#include "secodeFuzz.h"
#include "StageContext.h"
#include "common.h"
#include "ChangeJsonUtil.h"
#define private public
#include "CommandParser.h"
using namespace fuzztest;

namespace {
std::string g_moduleJsonNoModule = R"({
    "app" :
    {
        "apiReleaseType" : "Beta1"
    }
})";

std::string g_moduleJsonNoModuleName = R"({
    "module" :
    {
        "packageName" : "entry"
    }
})";

std::string g_moduleJsonNoModulePackageName = R"({
    "module" :
    {
        "name" : "entry"
    }
})";

std::string g_moduleJson = R"({
    "module" :
    {
        "name" : "entry",
        "type" : "entry",
        "description" : "$string:module_desc",
        "mainElement" : "EntryAbility",
        "deviceTypes" :
        [
            "phone",
            "tablet",
            "2in1"
        ],
        "deliveryWithInstall" : true,
        "installationFree" : false,
        "pages" : "$profile:main_pages",
        "extensionAbilities": [{"name": ""}],
        "packageName":  "entry",
        "virtualMachine" : "ark11.0.2.0",
        "compileMode" : "esmodule",
        "descriptionId" : 16777220
    }
})";

std::string g_pkgInfoJson = R"({
    "entry": {
        "packageName": "entry",
        "bundleName": "",
        "moduleName": "",
        "version": "",
        "entryPath": "src/main/",
        "isSO": false,
        "dependencyAlias": ""
    }
})";

TEST(SetPkgContextInfoFuzzTest, test_json)
{
    std::cout << "--> SetPkgContextInfoFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"SetPkgContextInfoFuzzTest", 0)
    {
        std::map<std::string, std::string> pkgContextMap;
        std::map<std::string, std::string> packageNameList;
        CommandParser::GetInstance().appResourcePath = ".";
        CommandParser::GetInstance().loaderJsonPath = "./loader.json";
        // 变化数据
        std::string filePath = "module.json";
        ChangeJsonUtil::WriteFile(filePath, g_moduleJsonNoModule);
        OHOS::Ide::StageContext::GetInstance().SetPkgContextInfo(pkgContextMap, packageNameList);
        ChangeJsonUtil::WriteFile(filePath, g_moduleJsonNoModuleName);
        OHOS::Ide::StageContext::GetInstance().SetPkgContextInfo(pkgContextMap, packageNameList);
        ChangeJsonUtil::WriteFile(filePath, g_moduleJsonNoModulePackageName);
        OHOS::Ide::StageContext::GetInstance().SetPkgContextInfo(pkgContextMap, packageNameList);
        uint64_t moduleIndex = 0;
        cJSON* moduleJsonArgs = cJSON_Parse(g_moduleJson.c_str());
        ChangeJsonUtil::ModifyObject(moduleJsonArgs, moduleIndex);
        ChangeJsonUtil::WriteFile(filePath, moduleJsonArgs);
        cJSON_Delete(moduleJsonArgs);
        std::string pkgContextFilePath = "pkgContextInfo.json";
        uint64_t pkgContextIndex = 0;
        cJSON* pkgContextJsonArgs = cJSON_Parse(g_pkgInfoJson.c_str());
        ChangeJsonUtil::ModifyObject(pkgContextJsonArgs, pkgContextIndex);
        ChangeJsonUtil::WriteFile(pkgContextFilePath, pkgContextJsonArgs);
        cJSON_Delete(pkgContextJsonArgs);
        // 执行接口
        OHOS::Ide::StageContext::GetInstance().SetPkgContextInfo(pkgContextMap, packageNameList);
    }
    DT_FUZZ_END()
    printf("end ---- SetPkgContextInfoFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test SetPkgContextInfoFuzzTest is not ok\r\n");
    } else {
        printf("test SetPkgContextInfoFuzzTest is ok\r\n");
    }
    std::cout << "--> SetPkgContextInfoFuzzTest for common end <--" << std::endl;
}
}