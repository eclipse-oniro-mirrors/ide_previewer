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
#include <sstream>
#include <limits>
#include <cstdint>
#include "PreviewerEngineLog.h"
#include "cJSON.h"

using namespace std;

namespace Json2 {
    Value::Value(cJSON* object) : jsonPtr(object), rootNode(true) {}

    Value::Value(cJSON* object, bool isRoot) : jsonPtr(object), rootNode(isRoot) {}

    Value::~Value()
    {
        if (!jsonPtr) {
            return;
        }
        if (rootNode) {
            cJSON_Delete(jsonPtr);
        }
        jsonPtr = nullptr;
    }

    Value Value::operator[](const char* key)
    {
        if (!cJSON_HasObjectItem(jsonPtr, key)) {
            return Value();
        }
        return Value(cJSON_GetObjectItemCaseSensitive(jsonPtr, key), false);
    }

    const Value Value::operator[](const char* key) const
    {
        if (!cJSON_HasObjectItem(jsonPtr, key)) {
            return Value();
        }
        return Value(cJSON_GetObjectItemCaseSensitive(jsonPtr, key), false);
    }

    Value Value::operator[](const std::string& key)
    {
        if (!cJSON_HasObjectItem(jsonPtr, key.c_str())) {
            return Value();
        }
        return Value(cJSON_GetObjectItemCaseSensitive(jsonPtr, key.c_str()), false);
    }

    const Value Value::operator[](const std::string& key) const
    {
        if (!cJSON_HasObjectItem(jsonPtr, key.c_str())) {
            return Value();
        }
        return Value(cJSON_GetObjectItemCaseSensitive(jsonPtr, key.c_str()), false);
    }

    Value::Members Value::GetMemberNames() const
    {
        Members names;
        if (jsonPtr) {
            cJSON* item = jsonPtr->child;
            while (item != nullptr) {
                names.push_back(item->string);
                item = item->next;
            }
        }
        return names;
    }

    std::string Value::ToString() const
    {
        std::string ret;
        if (!jsonPtr) {
            return ret;
        }
        char* jsonData = cJSON_PrintUnformatted(jsonPtr);
        if (jsonData) {
            ret = jsonData;
            cJSON_free(jsonData);
        }
        return ret;
    }

    std::string Value::ToStyledString() const
    {
        std::string ret;
        if (!jsonPtr) {
            return ret;
        }
        char* jsonData = cJSON_Print(jsonPtr);
        if (jsonData) {
            ret = jsonData;
            cJSON_free(jsonData);
        }
        return ret;
    }

    const cJSON* Value::GetJsonPtr() const
    {
        return jsonPtr;
    }

    bool Value::IsNull() const
    {
        return !jsonPtr || cJSON_IsNull(jsonPtr);
    }

    bool Value::IsValid() const
    {
        return jsonPtr && !cJSON_IsInvalid(jsonPtr);
    }

    bool Value::IsNumber() const
    {
        return cJSON_IsNumber(jsonPtr);
    }

    bool Value::IsInt() const
    {
        if (!IsNumber()) {
            return false;
        }
        double num = cJSON_GetNumberValue(jsonPtr);
        return (num >= static_cast<double>(std::numeric_limits<int32_t>::min())) &&
            (num <= static_cast<double>(std::numeric_limits<int32_t>::max()));
    }

    bool Value::IsUInt() const
    {
        if (!IsNumber()) {
            return false;
        }
        double num = cJSON_GetNumberValue(jsonPtr);
        return (num >= static_cast<double>(std::numeric_limits<uint32_t>::min())) &&
            (num <= static_cast<double>(std::numeric_limits<uint32_t>::max()));
    }

    bool Value::IsInt64() const
    {
        if (!IsNumber()) {
            return false;
        }
        double num = cJSON_GetNumberValue(jsonPtr);
        return (num >= static_cast<double>(std::numeric_limits<int64_t>::min())) &&
            (num <= static_cast<double>(std::numeric_limits<int64_t>::max()));
    }

    bool Value::IsUInt64() const
    {
        if (!IsNumber()) {
            return false;
        }
        double num = cJSON_GetNumberValue(jsonPtr);
        return (num >= static_cast<double>(std::numeric_limits<uint64_t>::min())) &&
            (num <= static_cast<double>(std::numeric_limits<uint64_t>::max()));
    }

    bool Value::IsDouble() const
    {
        if (!IsNumber()) {
            return false;
        }
        double num = cJSON_GetNumberValue(jsonPtr);
        return (num >= std::numeric_limits<double>::lowest()) && (num <= std::numeric_limits<double>::max());
    }

    bool Value::IsBool() const
    {
        return cJSON_IsBool(jsonPtr);
    }

    bool Value::IsString() const
    {
        return cJSON_IsString(jsonPtr);
    }

    bool Value::IsObject() const
    {
        return cJSON_IsObject(jsonPtr);
    }

    bool Value::IsArray() const
    {
        return cJSON_IsArray(jsonPtr);
    }

    bool Value::IsMember(const char* key) const
    {
        return cJSON_HasObjectItem(jsonPtr, key);
    }

    int32_t Value::GetInt(const char* key, int32_t defaultVal) const
    {
        return static_cast<int32_t>(GetDouble(key, defaultVal));
    }

    uint32_t Value::GetUInt(const char* key, int32_t defaultVal) const
    {
        return static_cast<uint32_t>(GetDouble(key, defaultVal));
    }
    
    int64_t Value::GetInt64(const char* key, int32_t defaultVal) const
    {
        return static_cast<int64_t>(GetDouble(key, defaultVal));
    }
    
    float Value::GetFloat(const char* key, float defaultVal) const
    {
        return static_cast<float>(GetDouble(key, defaultVal));
    }
    
    double Value::GetDouble(const char* key, double defaultVal) const
    {
        Value val = GetValue(key);
        if (!val.IsNull() && val.IsNumber()) {
            return val.AsDouble();
        }
        return defaultVal;
    }
    
    bool Value::GetBool(const char* key, bool defaultVal) const
    {
        Value val = GetValue(key);
        if (!val.IsNull() && val.IsBool()) {
            return val.AsBool();
        }
        return defaultVal;
    }
    
    std::string Value::GetString(const char* key, const std::string defaultVal) const
    {
        Value val = GetValue(key);
        if (!val.IsNull() && val.IsString()) {
            return val.AsString();
        }
        return defaultVal;
    }

    Value Value::GetValue(const char* key) const
    {
        return Value(cJSON_GetObjectItemCaseSensitive(jsonPtr, key), false);
    }

    int32_t Value::AsInt() const
    {
        return static_cast<int32_t>(AsDouble());
    }

    uint32_t Value::AsUInt() const
    {
        return static_cast<uint32_t>(AsDouble());
    }
    
    int64_t Value::AsInt64() const
    {
        return static_cast<int64_t>(AsDouble());
    }
    
    float Value::AsFloat() const
    {
        return static_cast<float>(AsDouble());
    }
    
    double Value::AsDouble() const
    {
        if (jsonPtr && cJSON_IsNumber(jsonPtr)) {
            return cJSON_GetNumberValue(jsonPtr);
        }
        return 0.0;
    }
    
    bool Value::AsBool() const
    {
        if (jsonPtr && cJSON_IsBool(jsonPtr)) {
            return cJSON_IsTrue(jsonPtr);
        }
        return false;
    }
    
    std::string Value::AsString() const
    {
        if (jsonPtr && cJSON_IsString(jsonPtr)) {
            return std::string(cJSON_GetStringValue(jsonPtr));
        }
        return "";
    }
    
    
    bool Value::Add(const char* key, const char* value)
    {
        if (!key || !cJSON_IsObject(jsonPtr)) {
            return false;
        }
        cJSON* child = cJSON_CreateString(value);
        if (child == nullptr) {
            return false;
        }
        cJSON_AddItemToObject(jsonPtr, key, child);
        return true;
    }

    bool Value::Add(const char* key, bool value)
    {
        if (!key || !cJSON_IsObject(jsonPtr)) {
            return false;
        }
        cJSON* child = cJSON_CreateBool(static_cast<int>(value));
        if (child == nullptr) {
            return false;
        }
        cJSON_AddItemToObject(jsonPtr, key, child);
        return true;
    }
    
    bool Value::Add(const char* key, int32_t value)
    {
        return Add(key, static_cast<double>(value));
    }
    
    bool Value::Add(const char* key, uint32_t value)
    {
        return Add(key, static_cast<double>(value));
    }
    
    bool Value::Add(const char* key, int64_t value)
    {
        return Add(key, static_cast<double>(value));
    }
    
    bool Value::Add(const char* key, double value)
    {
        if (!key || !cJSON_IsObject(jsonPtr)) {
            return false;
        }
        cJSON* child = cJSON_CreateNumber(value);
        if (child == nullptr) {
            return false;
        }
        cJSON_AddItemToObject(jsonPtr, key, child);
        return true;
    }
    
    bool Value::Add(const char* key, const Value& value)
    {
        if (!key || value.IsNull() || !value.IsValid()) {
            return false;
        }
        cJSON* jsonObject = cJSON_Duplicate(const_cast<cJSON*>(value.GetJsonPtr()), true);
        if (jsonObject == nullptr) {
            return false;
        }
        cJSON_AddItemToObject(jsonPtr, key, jsonObject);
        return true;
    }

    bool Value::Add(const char* value)
    {
        if (!cJSON_IsArray(jsonPtr)) {
            return false;
        }
        cJSON* child = cJSON_CreateString(value);
        if (child == nullptr) {
            return false;
        }
        cJSON_AddItemToArray(jsonPtr, child);
        return true;
    }

    bool Value::Add(bool value)
    {
        if (!cJSON_IsArray(jsonPtr)) {
            return false;
        }
        cJSON* child = cJSON_CreateBool(static_cast<int>(value));
        if (child == nullptr) {
            return false;
        }
        cJSON_AddItemToArray(jsonPtr, child);
        return true;
    }
    
    bool Value::Add(int32_t value)
    {
        return Add(static_cast<double>(value));
    }
    
    bool Value::Add(uint32_t value)
    {
        return Add(static_cast<double>(value));
    }
    
    bool Value::Add(int64_t value)
    {
        return Add(static_cast<double>(value));
    }
    
    bool Value::Add(double value)
    {
        if (!cJSON_IsArray(jsonPtr)) {
            return false;
        }
        cJSON* child = cJSON_CreateNumber(value);
        if (child == nullptr) {
            return false;
        }
        cJSON_AddItemToArray(jsonPtr, child);
        return true;
    }
    
    
    bool Value::Add(const Value& value)
    {
        if (value.IsNull() || !value.IsValid()) {
            return false;
        }
        cJSON* jsonObject = cJSON_Duplicate(const_cast<cJSON*>(value.GetJsonPtr()), true);
        if (jsonObject == nullptr) {
            return false;
        }
        cJSON_AddItemToArray(jsonPtr, jsonObject);
        return true;
    }

    bool Value::Replace(const char* key, bool value)
    {
        if (!key) {
            return false;
        }
        cJSON* child = cJSON_CreateBool(static_cast<int>(value));
        if (child == nullptr) {
            return false;
        }
        if (!cJSON_ReplaceItemInObjectCaseSensitive(jsonPtr, key, child)) {
            cJSON_Delete(child);
            return false;
        }
        return true;
    }

    bool Value::Replace(const char* key, int32_t value)
    {
        return Replace(key, static_cast<double>(value));
    }
    
    bool Value::Replace(const char* key, uint32_t value)
    {
        return Replace(key, static_cast<double>(value));
    }
    
    bool Value::Replace(const char* key, int64_t value)
    {
        return Replace(key, static_cast<double>(value));
    }
    
    bool Value::Replace(const char* key, double value)
    {
        if (!key) {
            return false;
        }
        cJSON* child = cJSON_CreateNumber(value);
        if (child == nullptr) {
            return false;
        }
        if (!cJSON_ReplaceItemInObjectCaseSensitive(jsonPtr, key, child)) {
            cJSON_Delete(child);
            return false;
        }
        return true;
    }
    
    bool Value::Replace(const char* key, const char* value)
    {
        if (!key) {
            return false;
        }
        cJSON* child = cJSON_CreateString(value);
        if (child == nullptr) {
            return false;
        }
        if (!cJSON_ReplaceItemInObjectCaseSensitive(jsonPtr, key, child)) {
            cJSON_Delete(child);
            return false;
        }
        return true;
    }
    
    bool Value::Replace(const char* key, const Value& value)
    {
        if (!key) {
            return false;
        }
        cJSON* jsonObject = cJSON_Duplicate(const_cast<cJSON*>(value.GetJsonPtr()), true);
        if (jsonObject == nullptr) {
            return false;
        }

        if (!cJSON_ReplaceItemInObjectCaseSensitive(jsonPtr, key, jsonObject)) {
            cJSON_Delete(jsonObject);
            return false;
        }
        return true;
    }

    bool Value::Replace(int index, bool value)
    {
        if (index < 0 || index >= GetArraySize()) {
            return false;
        }
        cJSON* child = cJSON_CreateBool(static_cast<int>(value));
        if (child == nullptr) {
            return false;
        }
        if (!cJSON_ReplaceItemInArray(jsonPtr, index, child)) {
            cJSON_Delete(child);
            return false;
        }
        return true;
    }

    bool Value::Replace(int index, int32_t value)
    {
        return Replace(index, static_cast<double>(value));
    }
    
    bool Value::Replace(int index, uint32_t value)
    {
        return Replace(index, static_cast<double>(value));
    }
    
    bool Value::Replace(int index, int64_t value)
    {
        return Replace(index, static_cast<double>(value));
    }
    
    bool Value::Replace(int index, double value)
    {
        if (index < 0 || index >= GetArraySize()) {
            return false;
        }
        cJSON* child = cJSON_CreateNumber(value);
        if (child == nullptr) {
            return false;
        }
        if (!cJSON_ReplaceItemInArray(jsonPtr, index, child)) {
            cJSON_Delete(child);
            return false;
        }
        return true;
    }
    
    bool Value::Replace(int index, const char* value)
    {
        if (index < 0 || index >= GetArraySize()) {
            return false;
        }
        cJSON* child = cJSON_CreateString(value);
        if (child == nullptr) {
            return false;
        }
        if (!cJSON_ReplaceItemInArray(jsonPtr, index, child)) {
            cJSON_Delete(child);
            return false;
        }
        return true;
    }
    
    bool Value::Replace(int index, const Value& value)
    {
        if (index < 0 || index >= GetArraySize()) {
            return false;
        }
        cJSON* jsonObject = cJSON_Duplicate(const_cast<cJSON*>(value.GetJsonPtr()), true);
        if (jsonObject == nullptr) {
            return false;
        }

        if (!cJSON_ReplaceItemInArray(jsonPtr, index, jsonObject)) {
            cJSON_Delete(jsonObject);
            return false;
        }
        return true;
    }

    uint32_t Value::GetArraySize() const
    {
        return cJSON_GetArraySize(jsonPtr);
    }

    Value Value::GetArrayItem(int32_t index) const
    {
        return Value(cJSON_GetArrayItem(jsonPtr, index), false);
    }

    void Value::Clear()
    {
        cJSON_Delete(jsonPtr);
        jsonPtr = cJSON_CreateObject();
    }

    std::string Value::GetKey()
    {
        const char* key = jsonPtr->string;
        if (key) {
            return std::string(key);
        }
        return string();
    }
}


string JsonReader::ReadFile(const string& path)
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

Json2::Value JsonReader::ParseJsonData2(const std::string& jsonStr)
{
    return Json2::Value(cJSON_Parse(jsonStr.c_str()));
}

std::string JsonReader::GetErrorPtr()
{
    const char* err = cJSON_GetErrorPtr();
    if (err) {
        return std::string(err);
    }
    return string();
}

Json2::Value JsonReader::CreateObject()
{
    return Json2::Value(cJSON_CreateObject());
}

Json2::Value JsonReader::CreateArray()
{
    return Json2::Value(cJSON_CreateArray());
}

Json2::Value JsonReader::CreateBool(const bool value)
{
    return Json2::Value(cJSON_CreateBool(value));
}

Json2::Value JsonReader::CreateString(const std::string& value)
{
    return Json2::Value(cJSON_CreateString(value.c_str()));
}

Json2::Value JsonReader::DepthCopy(const Json2::Value& value)
{
    return Json2::Value(cJSON_Duplicate(const_cast<cJSON*>(value.GetJsonPtr()), true));
}

Json2::Value JsonReader::CreateNull()
{
    return Json2::Value(cJSON_CreateNull());
}