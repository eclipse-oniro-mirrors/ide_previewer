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

#ifndef CHANGE_JSON_UTIL_H
#define CHANGE_JSON_UTIL_H

#include <string>
#include <vector>
#include <map>

struct cJSON;

namespace fuzztest {
    class ChangeJsonUtil {
    public:
        static void ModifyObject(cJSON *object, uint64_t& idx);
        static void ModifyObject4ChangeType(cJSON *object, uint64_t& idx);
        static void WriteFile(std::string filePath, cJSON *object);
        static void WriteFile(std::string filePath, std::string str);
    };
}

#endif  // CHANGE_JSON_UTIL_H