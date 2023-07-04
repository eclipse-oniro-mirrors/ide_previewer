/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef JSONREADER_H
#define JSONREADER_H

#include <string>

#include "json.h"

class JsonReader {
public:
    static std::string ReadFile(const std::string path);
    static Json::Value ParseJsonData(const std::string jsonStr);

    static std::string GetString(const Json::Value& val, const std::string& key,
                                const std::string& defaultVal = "");
    static bool GetBool(const Json::Value& val, const std::string& key, const bool defaultVal = false);
    static int32_t GetInt(const Json::Value& val, const std::string& key, const int32_t defaultVal = 0);
    static uint32_t GetUInt(const Json::Value& val, const std::string& key, const uint32_t defaultVal = 0);
    static int64_t GetInt64(const Json::Value& val, const std::string& key, const int64_t defaultVal = 0);
    static double GetDouble(const Json::Value& val, const std::string& key, const double defaultVal = 0.0);
    static std::unique_ptr<Json::Value> GetObject(const Json::Value& val, const std::string& key);
    static int32_t GetArraySize(const Json::Value& val);
    static std::unique_ptr<Json::Value> GetArray(const Json::Value& val, const std::string& key);
};

#endif // JSONREADER_H
