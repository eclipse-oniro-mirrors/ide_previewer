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

#include "JsonReader.h"

#include <fstream>

#include "PreviewerEngineLog.h"
#include "json/json.h"

using namespace std;
string JsonReader::ReadFile(const string path)
{
    ifstream inFile(path);
    if (!inFile.is_open()) {
        ELOG("JsonReader: Open json file failed.");
        return string();
    }
    string jsonStr((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();
    return jsonStr;
}

Json::Value JsonReader::ParseJsonData(const string jsonStr)
{
    Json::Value val;
    Json::CharReaderBuilder builder;
    Json::CharReader* charReader = builder.newCharReader();
    if (charReader == nullptr) {
        ELOG("JsonReader: CharReader memory allocation failed.");
        return val;
    }
    unique_ptr<Json::CharReader> reader(charReader);
    if (reader.get() == nullptr) {
        ELOG("JsonReader: CharReader get null.");
        return val;
    }
    string message; // NOLINT
    if (!reader->parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), &val, &message)) {
        ELOG("JsonReader: Failed to parse the json data, errors: %s", message.c_str());
    }
    return val;
}

string JsonReader::GetString(const Json::Value& val, const string& key, const string& defaultVal)
{
    if (val && val.isMember(key) && val[key].isString()) {
        return val[key].asString();
    }
    ELOG("JsonReader: GetString failed, key is: %s.", key.c_str());
    return defaultVal;
}

bool JsonReader::GetBool(const Json::Value& val, const string& key, const bool defaultVal)
{
    if (val && val.isMember(key) && val[key].isBool()) {
        return val[key].asBool();
    }
    ELOG("JsonReader: GetBool failed, key is: %s.", key.c_str());
    return defaultVal;
}

int32_t JsonReader::GetInt(const Json::Value& val, const string& key, const int32_t defaultVal)
{
    if (val && val.isMember(key) && val[key].isInt()) {
        return val[key].asInt();
    }
    ELOG("JsonReader: GetInt failed, key is: %s.", key.c_str());
    return defaultVal;
}

uint32_t JsonReader::GetUInt(const Json::Value& val, const string& key, const uint32_t defaultVal)
{
    if (val && val.isMember(key) && val[key].isUInt()) {
        return val[key].asUInt();
    }
    ELOG("JsonReader: GetUInt failed, key is: %s.", key.c_str());
    return defaultVal;
}

int64_t JsonReader::GetInt64(const Json::Value& val, const string& key, const int64_t defaultVal)
{
    if (val && val.isMember(key) && val[key].isInt64()) {
        return val[key].asInt64();
    }
    ELOG("JsonReader: GetInt64 failed, key is: %s.", key.c_str());
    return defaultVal;
}

double JsonReader::GetDouble(const Json::Value& val, const string& key, const double defaultVal)
{
    if (val && val.isMember(key) && val[key].isDouble()) {
        return val[key].asDouble();
    }
    ELOG("JsonReader: GetDouble failed, key is: %s.", key.c_str());
    return defaultVal;
}

unique_ptr<Json::Value> JsonReader::GetObject(const Json::Value& val, const string& key)
{
    if (val && val.isMember(key) && val[key].isObject()) {
        return make_unique<Json::Value>(val[key]);
    }
    ELOG("JsonReader: GetObject failed, key is: %s.", key.c_str());
    return make_unique<Json::Value>();
}

int32_t JsonReader::GetArraySize(const Json::Value& val)
{
    if (val && val.isArray()) {
        return val.size();
    }
    ELOG("JsonReader: GetArraySize failed.");
    return 0;
}

unique_ptr<Json::Value> JsonReader::GetArray(const Json::Value& val, const string& key)
{
    if (val && val.isMember(key) && val[key].isArray()) {
        return make_unique<Json::Value>(val[key]);
    }
    ELOG("JsonReader: GetArraySize failed, key is: %s.", key.c_str());
    return make_unique<Json::Value>();
}