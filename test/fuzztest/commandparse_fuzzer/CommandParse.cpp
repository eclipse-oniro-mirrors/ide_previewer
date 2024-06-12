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
using namespace std;
using namespace fuzztest;

const int DEFAULT_LENGTH = 1000;
const int DEFAULT_INT = 1000;
namespace {
const std::string DEFAULT_STRING = "aaaa";

void modifyObject(cJSON *object, uint64_t& idx)
{
    if (!object) {
        return;
    }
    cJSON *item = nullptr;
    cJSON_ArrayForEach(item, object) {
        if (item->type == cJSON_String) {
            std::string strVal = DT_SetGetString(&g_Element[idx], strlen(item->valuestring) + 1, DEFAULT_LENGTH,
                (char*)item->valuestring);
            idx++;
            cJSON_free(item->valuestring); // 释放原字符串内存
            item->valuestring = nullptr;
            int length = strVal.length() + 1;
            item->valuestring = (char*)malloc(length * sizeof(char)); // 分配内存
            errno_t ret1 = strcpy_s(item->valuestring, length, strVal.c_str()); // 复制字符串内容
            if (ret1 != EOK) {
                printf("strcpy_s in modifyObject copy error");
            }
        } else if (item->type == cJSON_Number) {
            int intVal = *(s32 *)DT_SetGetS32(&g_Element[idx], item->valueint);
            idx++;
            item->valueint = intVal;
        } else if (item->type == cJSON_True || item->type == cJSON_False) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::bernoulli_distribution distribution(0.5); // 0.5 is probability to generate true or false
            bool ret = distribution(gen);
            item->type = ret ? cJSON_True : cJSON_False;
        } else if (item->type == cJSON_Object) {
            cJSON *arrayItem = nullptr;
            cJSON_ArrayForEach(arrayItem, item) {
                modifyObject(arrayItem, idx);
            }
        } else if (item->type == cJSON_Array) {
            cJSON *objItem = nullptr;
            cJSON_ArrayForEach(objItem, item) {
                modifyObject(objItem, idx);
            }
        }
    }
}

void modifyObject4ChangeType(cJSON *object, uint64_t& idx)
{
    if (!object) {
        return;
    }
    cJSON *item = nullptr;
    cJSON_ArrayForEach(item, object) {
        if (item->type == cJSON_String) {
            int32_t intValue = *(s32 *)DT_SetGetS32(&g_Element[idx], DEFAULT_INT);
            idx++;
            item->type = cJSON_Number;
            item->valuedouble = (double)intValue;
            item->valueint = intValue;
            cJSON_free(item->valuestring); // 释放原字符串内存
            item->valuestring = nullptr;
        } else if (item->type == cJSON_Number || item->type == cJSON_True || item->type == cJSON_False) {
            std::string strVal = DT_SetGetString(&g_Element[idx], DEFAULT_STRING.size() + 1, DEFAULT_LENGTH,
                (char*)DEFAULT_STRING.c_str());
            idx++;
            item->type = cJSON_String;
            int length = strVal.length() + 1;
            item->valuestring = (char*)malloc(length * sizeof(char)); // 分配内存
            errno_t ret2 = strcpy_s(item->valuestring, length, strVal.c_str()); // 复制字符串内容
            if (ret2 != EOK) {
                printf("strcpy_s in modifyObject4ChangeType copy error");
            }
        } else if (item->type == cJSON_Object) {
            cJSON *arrayItem = nullptr;
            cJSON_ArrayForEach(arrayItem, item) {
                modifyObject4ChangeType(arrayItem, idx);
            }
        } else if (item->type == cJSON_Array) {
            cJSON *objItem = nullptr;
            cJSON_ArrayForEach(objItem, item) {
                modifyObject4ChangeType(objItem, idx);
            }
        }
    }
}
}

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
            modifyObject(jsonArgs, index);
        } else {
            modifyObject4ChangeType(jsonArgs, index);
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
    for (map<string, string>::iterator iter = dataMap.begin(); iter != dataMap.end(); iter++) {
        for (int j = 0; j < types.size(); j++) {
            std::string key = iter->first;
            std::string val = iter->second;
            Execute(key, val, j, index, false);
            Execute(key, val, j, index, true);
        }
    }
}