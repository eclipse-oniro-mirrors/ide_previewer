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
#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>

struct cJSON;

namespace Json2 {
    class Value {
    public:
        Value() = default;
        explicit Value(cJSON* object);
        Value(cJSON* object, bool isRoot);
        ~Value();
        // 重载实现obj["key"]形式调用
        Value operator[](const char* key);
        const Value operator[](const char* key) const;
        Value operator[](const std::string& key);
        const Value operator[](const std::string& key) const;
        // 获取所有成员键值
        using Members = std::vector<std::string>;
        Value::Members GetMemberNames() const;
        // convert string functions
        std::string ToString() const;
        std::string ToStyledString() const;
        const cJSON* GetJsonPtr() const;
        // check functions
        bool IsNull() const;
        bool IsValid() const;
        bool IsNumber() const;
        bool IsInt() const;
        bool IsUInt() const;
        bool IsInt64() const;
        bool IsUInt64() const;
        bool IsDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsObject() const;
        bool IsArray() const;
        bool IsMember(const char* key) const;
        // get functions
        int32_t GetInt(const char* key, int32_t defaultVal = 0) const;
        uint32_t GetUInt(const char* key, int32_t defaultVal = 0) const;
        int64_t GetInt64(const char* key, int32_t defaultVal = 0) const;
        float GetFloat(const char* key, float defaultVal = 0.0) const;
        double GetDouble(const char* key, double defaultVal = 0.0) const;
        bool GetBool(const char* key, bool defaultVal = false) const;
        std::string GetString(const char* key, std::string defaultVal = "") const;
        Value GetValue(const char* key) const;
        int32_t AsInt() const;
        uint32_t AsUInt() const;
        int64_t AsInt64() const;
        float AsFloat() const;
        double AsDouble() const;
        bool AsBool() const;
        std::string AsString() const;
        // add functions for obj
        bool Add(const char* key, const char* value);
        bool Add(const char* key, bool value);
        bool Add(const char* key, int32_t value);
        bool Add(const char* key, uint32_t value);
        bool Add(const char* key, int64_t value);
        bool Add(const char* key, double value);
        bool Add(const char* key, const Value& value);
        // add functions for array
        bool Add(const char* value);
        bool Add(bool value);
        bool Add(int32_t value);
        bool Add(uint32_t value);
        bool Add(int64_t value);
        bool Add(double value);
        bool Add(const Value& value);
        // replace functions for obj
        bool Replace(const char* key, bool value);
        bool Replace(const char* key, int32_t value);
        bool Replace(const char* key, uint32_t value);
        bool Replace(const char* key, int64_t value);
        bool Replace(const char* key, double value);
        bool Replace(const char* key, const char* value);
        bool Replace(const char* key, const Value& value);
        // replace functions for array
        bool Replace(int index, bool value);
        bool Replace(int index, int32_t value);
        bool Replace(int index, uint32_t value);
        bool Replace(int index, int64_t value);
        bool Replace(int index, double value);
        bool Replace(int index, const char* value);
        bool Replace(int index, const Value& value);
        // array functions
        uint32_t GetArraySize() const;
        Value GetArrayItem(int32_t index) const;
        // empty object
        void Clear();
        std::string GetKey();

    private:
        cJSON* jsonPtr = nullptr;
        bool rootNode = true;
    };
}

class JsonReader {
public:
    static std::string ReadFile(const std::string& path);
    static Json2::Value ParseJsonData2(const std::string& jsonStr);
    static std::string GetErrorPtr();
    static Json2::Value CreateObject();
    static Json2::Value CreateArray();
    static Json2::Value CreateBool(const bool value);
    static Json2::Value CreateString(const std::string& value);
    static Json2::Value DepthCopy(const Json2::Value& value);
    static Json2::Value CreateNull();
};

#endif // JSONREADER_H
