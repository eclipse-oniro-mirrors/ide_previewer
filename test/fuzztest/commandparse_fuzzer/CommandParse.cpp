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

#include "CommandParse.h"
#include <random>
#include "secodeFuzz.h"
#include "cJSON.h"
#include "securec.h"
#include "CommandLineInterface.h"
#include "ChangeJsonUtil.h"
using namespace fuzztest;

void CommandParse::Execute(std::string& commond, std::string& jsonArgsStr,
    int& typeIndex, uint64_t& index, bool changeType)
{
    // 变化数据，不改变数据类型
    cJSON* jsonData = cJSON_CreateObject();
    if (jsonData == nullptr) {
        return;
    }
    cJSON_AddItemToObject(jsonData, "version", cJSON_CreateString("1.0.1"));
    cJSON_AddItemToObject(jsonData, "command", cJSON_CreateString(commond.c_str()));
    cJSON_AddItemToObject(jsonData, "type", cJSON_CreateString(types[typeIndex].c_str()));
    if (!jsonArgsStr.empty()) {
        cJSON* jsonArgs = cJSON_Parse(jsonArgsStr.c_str());
        if (!changeType) {
            ChangeJsonUtil::ModifyObject(jsonArgs, index);
        } else {
            ChangeJsonUtil::ModifyObject4ChangeType(jsonArgs, index);
        }
        cJSON_AddItemToObject(jsonData, "args", jsonArgs);
    }
    char* dataStr = cJSON_PrintUnformatted(jsonData);
    printf("============================command1:%s\r\n", dataStr);
    std::string dataString = std::string(dataStr);
    free(dataStr);
    CommandLineInterface::GetInstance().ProcessCommandMessage(dataString);
    cJSON_Delete(jsonData);
}

void CommandParse::CreateAndExecuteCommand(std::map<std::string, std::string> dataMap)
{
    CommandLineInterface::GetInstance().Init("pipeName");
    uint64_t index = 0;
    for (std::map<std::string, std::string>::iterator iter = dataMap.begin(); iter != dataMap.end(); iter++) {
        for (int j = 0; j < types.size(); j++) {
            std::string key = iter->first;
            std::string val = iter->second;
            Execute(key, val, j, index, false);
            Execute(key, val, j, index, true);
        }
    }
}