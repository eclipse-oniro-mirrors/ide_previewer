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
#include "ChangeJsonUtil.h"
#include "options.h"
#define private public
#include "CommandParser.h"
#include "JsAppImpl.h"
using namespace std;
using namespace fuzztest;

namespace {
std::string g_ModuleJson_NoModule = R"({
    "app" :
    {
        "apiReleaseType" : "Beta1"
    }
})";

std::string g_ModuleJson_NoModuleName = R"({
    "module" :
    {
        "packageName" : "entry"
    }
})";

std::string g_ModuleJson_NoModulePackageName = R"({
    "module" :
    {
        "name" : "entry"
    }
})";

std::string g_ModuleJson = R"({
    "app" :
    {
        "apiReleaseType" : "Beta1",
        "bundleName" : "com.example.myapplication",
        "bundleType" : "app",
        "compileSdkType" : "XxxOS",
        "compileSdkVersion" : "4.1.0.52",
        "debug" : true,
        "icon" : "$media:app_icon",
        "iconId" : 16777217,
        "label" : "$string:app_name",
        "labelId" : 16777216,
        "minAPIVersion" : 40100011,
        "targetAPIVersion" : 40100011,
        "vendor" : "example",
        "versionCode" : 1000000,
        "versionName" : "1.0.0"
    },
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
        "abilities" :
        [
            {
                "description" : "$string:EntryAbility_desc",
                "descriptionId" : 16777218,
                "exported" : true,
                "icon" : "$media:icon",
                "iconId" : 16777224,
                "label" : "$string:EntryAbility_label",
                "labelId" : 16777219,
                "name" : "EntryAbility",
                "skills" :
                [
                    {
                        "actions" :
                        [
                            "action.system.home"
                        ],
                        "entities" :
                        [
                            "entity.system.home"
                        ]
                    }
                ],
                "srcEntry" : "./ets/entryability/EntryAbility.ets",
                "startWindowBackground" : "$color:start_window_background",
                "startWindowBackgroundId" : 16777222,
                "startWindowIcon" : "$media:startIcon",
                "startWindowIconId" : 16777223
            }
        ],
        "extensionAbilities": [{"name": ""}],
        "packageName":  "entry",
        "virtualMachine" : "ark11.0.2.0",
        "compileMode" : "esmodule",
        "dependencies" :
        [
            {
                "moduleName" : "library"
            }
        ],
        "descriptionId" : 16777220
    }
})";

std::string g_PkgInfoJson = R"({
    "entry": {
        "packageName": "entry",
        "bundleName": "",
        "moduleName": "",
        "version": "",
        "entryPath": "src/main/",
        "isSO": false,
        "dependencyAlias": ""
    },
    "@ohos/hypium": {
        "packageName": "@ohos/hypium",
        "bundleName": "",
        "moduleName": "",
        "version": "1.0.18",
        "entryPath": "index.js",
        "isSO": false,
        "dependencyAlias": ""
    },
    "@ohos/hamock": {
        "packageName": "@ohos/hamock",
        "bundleName": "",
        "moduleName": "",
        "version": "1.0.1-rc2",
        "entryPath": "index.ets",
        "isSO": false,
        "dependencyAlias": ""
    }
})";

TEST(PkgContextInfoParseTest, test_json)
{
    std::cout << "--> PkgContextInfoParseTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"PkgContextInfoParseTest", 0)
    {
        CommandParser::GetInstance().appResourcePath = ".";
        CommandParser::GetInstance().loaderJsonPath = ".";
        JsAppImpl::GetInstance().InitCommandInfo();
        JsAppImpl::GetInstance().SetPkgContextInfo();
        // 变化数据
        std::string filePath = "module.json";
        ChangeJsonUtil::WriteFile(filePath, g_ModuleJson_NoModule);
        JsAppImpl::GetInstance().SetPkgContextInfo();
        ChangeJsonUtil::WriteFile(filePath, g_ModuleJson_NoModuleName);
        JsAppImpl::GetInstance().SetPkgContextInfo();
        ChangeJsonUtil::WriteFile(filePath, g_ModuleJson_NoModulePackageName);
        JsAppImpl::GetInstance().SetPkgContextInfo();
        uint64_t moduleIndex = 0;
        cJSON* moduleJsonArgs = cJSON_Parse(g_ModuleJson.c_str());
        ChangeJsonUtil::ModifyObject(moduleJsonArgs, moduleIndex);
        ChangeJsonUtil::WriteFile(filePath, moduleJsonArgs);
        cJSON_Delete(moduleJsonArgs);
        std::string pkgContextFilePath = "pkgContextInfo.json";
        uint64_t pkgContextIndex = 0;
        cJSON* pkgContextJsonArgs = cJSON_Parse(pkgContextFilePath.c_str());
        ChangeJsonUtil::ModifyObject(pkgContextJsonArgs, pkgContextIndex);
        ChangeJsonUtil::WriteFile(filePath, pkgContextJsonArgs);
        cJSON_Delete(pkgContextJsonArgs);
        // 执行接口
        JsAppImpl::GetInstance().SetPkgContextInfo();
    }
    DT_FUZZ_END()
    printf("end ---- PkgContextInfoParseTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test PkgContextInfoParseTest is not ok\r\n");
    } else {
        printf("test PkgContextInfoParseTest is ok\r\n");
    }
    std::cout << "--> PkgContextInfoParseTest for common end <--" << std::endl;
}
}