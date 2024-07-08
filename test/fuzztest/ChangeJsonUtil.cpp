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

#include "ChangeJsonUtil.h"
#include <random>
#include <fstream>
#include <iostream>
#include "secodeFuzz.h"
#include "cJSON.h"
#include "securec.h"
using namespace std;
using namespace fuzztest;

namespace {
const int DEFAULT_LENGTH = 1000;
const int DEFAULT_INT = 1000;
const std::string DEFAULT_STRING = "aaaa";
}

void ChangeJsonUtil::ModifyObject(cJSON *object, uint64_t& idx)
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
            item->valuestring = reinterpret_cast<char*>(malloc(length * sizeof(char))); // 分配内存
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
                ModifyObject(arrayItem, idx);
            }
        } else if (item->type == cJSON_Array) {
            cJSON *objItem = nullptr;
            cJSON_ArrayForEach(objItem, item) {
                ModifyObject(objItem, idx);
            }
        }
    }
}

void ChangeJsonUtil::ModifyObject4ChangeType(cJSON *object, uint64_t& idx)
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
            item->valuestring = reinterpret_cast<char*>(malloc(length * sizeof(char))); // 分配内存
            errno_t ret2 = strcpy_s(item->valuestring, length, strVal.c_str()); // 复制字符串内容
            if (ret2 != EOK) {
                printf("strcpy_s in modifyObject4ChangeType copy error");
            }
        } else if (item->type == cJSON_Object) {
            cJSON *arrayItem = nullptr;
            cJSON_ArrayForEach(arrayItem, item) {
                ModifyObject4ChangeType(arrayItem, idx);
            }
        } else if (item->type == cJSON_Array) {
            cJSON *objItem = nullptr;
            cJSON_ArrayForEach(objItem, item) {
                ModifyObject4ChangeType(objItem, idx);
            }
        }
    }
}

void ChangeJsonUtil::WriteFile(std::string filePath, cJSON *object)
{
    // Check if cJSON object is valid
    if (object == nullptr) {
        std::cerr << "Error: cJSON object is null." << std::endl;
        return;
    }

    // Open file stream
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    // Convert cJSON object to string
    char *jsonStr = cJSON_Print(object);
    if (jsonStr == nullptr) {
        std::cerr << "Error converting cJSON object to string." << std::endl;
        outFile.close();
        return;
    }

    // Write JSON string to file
    outFile << jsonStr << std::endl;

    // Clean up
    cJSON_free(jsonStr);
    outFile.close();

    std::cout << "JSON content successfully written to file: " << filePath << std::endl;
}

void ChangeJsonUtil::WriteFile(std::string filePath, std::string str)
{
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }
    outFile << str << std::endl;
    std::cout << "Str content successfully written to file: " << filePath << std::endl;
}