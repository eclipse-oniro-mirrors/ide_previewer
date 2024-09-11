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
#include <fstream>
#include "gtest/gtest.h"
#include "JsonReader.h"

namespace {
    std::string g_name = "jin";
    bool g_isChild = true;
    int32_t g_age = 13;
    uint32_t g_code1 = 214748;
    int64_t g_code2 = 3333333333;
    double g_height = 165.3;
    std::string g_schoolName = "abc";
    std::string g_schoolAddr = "cba";
    std::string g_newName = "li";
    bool g_newIsChild = false;
    int32_t g_newAge = 26;
    uint32_t g_newCode1 = 2147489;
    int64_t g_newCode2 = 3333333339;
    double g_newHeight = 185.3;
    std::string g_obj = R"({"name":"jin","isChild":true,"age":13,"code1":214748,"code2":3333333333,
        "height":165.3,"school":{"schoolName":"abc","schoolAddr":"cba"}})";
    std::string g_arr = "[\"jin\",true,13,214748,3333333333,165.3,{\"schoolName\":\"abc\",\"schoolAddr\":\"cba\"}]";

    TEST(JsonReaderTest, ParseJsonData2Test)
    {
        int age = 20;
        std::string str = "{\"name\":\"jin\", \"age\":20}";
        Json2::Value val = JsonReader::ParseJsonData2(str);
        EXPECT_EQ(val["name"].AsString(), "jin");
        EXPECT_EQ(val["age"].AsInt(), age);
    }

    TEST(JsonReaderTest, CreateObjectTest)
    {
        Json2::Value school = JsonReader::CreateObject();
        school.Add("schoolName", g_schoolName.c_str());
        school.Add("schoolAddr", g_schoolAddr.c_str());
        Json2::Value student = JsonReader::CreateObject();
        student.Add("name", g_name.c_str());
        EXPECT_EQ(student["name"].AsString(), g_name);
        student.Add("isChild", g_isChild);
        EXPECT_EQ(student["isChild"].AsBool(), g_isChild);
        student.Add("age", g_age);
        EXPECT_EQ(student["age"].AsInt(), g_age);
        student.Add("code1", g_code1);
        EXPECT_EQ(student["code1"].AsUInt(), g_code1);
        student.Add("code2", g_code2);
        EXPECT_EQ(student["code2"].AsInt64(), g_code2);
        student.Add("height", g_height);
        EXPECT_EQ(student["height"].AsDouble(), g_height);
        student.Add("school", school);
        EXPECT_EQ(student["school"]["schoolName"].AsString(), g_schoolName);
        EXPECT_EQ(student["school"]["schoolAddr"].AsString(), g_schoolAddr);
    }

    TEST(JsonReaderTest, CreateArrayTest)
    {
        Json2::Value school = JsonReader::CreateObject();
        school.Add("schoolName", g_schoolName.c_str());
        school.Add("schoolAddr", g_schoolAddr.c_str());
        Json2::Value books = JsonReader::CreateArray();
        books.Add(g_name.c_str());
        books.Add(g_isChild);
        books.Add(g_age);
        books.Add(g_code1);
        books.Add(g_code2);
        books.Add(g_height);
        books.Add(school);
        int size = 7;
        int actualSize = books.GetArraySize();
        EXPECT_EQ(actualSize, size);
        int index = 0;
        EXPECT_EQ(books.GetArrayItem(index++).AsString(), g_name);
        EXPECT_EQ(books.GetArrayItem(index++).AsBool(), g_isChild);
        EXPECT_EQ(books.GetArrayItem(index++).AsInt(), g_age);
        EXPECT_EQ(books.GetArrayItem(index++).AsUInt(), g_code1);
        EXPECT_EQ(books.GetArrayItem(index++).AsInt64(), g_code2);
        EXPECT_EQ(books.GetArrayItem(index++).AsDouble(), g_height);
        EXPECT_EQ(books.GetArrayItem(index)["schoolName"].AsString(), g_schoolName);
        EXPECT_EQ(books.GetArrayItem(index)["schoolAddr"].AsString(), g_schoolAddr);
    }

    TEST(JsonReaderTest, CreateBoolTest)
    {
        Json2::Value resultJson = JsonReader::CreateObject();
        resultJson.Add("result", JsonReader::CreateBool(true));
        EXPECT_TRUE(resultJson["result"].AsBool());
    }

    TEST(JsonReaderTest, CreateStringTest)
    {
        std::string content = "content";
        Json2::Value resultJson = JsonReader::CreateObject();
        resultJson.Add("content", JsonReader::CreateString(content));
        EXPECT_EQ(resultJson["content"].AsString(), content);
    }

    TEST(JsonReaderTest, DepthCopyTest)
    {
        std::string content = "content";
        Json2::Value resultJson = JsonReader::CreateObject();
        resultJson.Add("content", JsonReader::CreateString(content));
        Json2::Value newJson = JsonReader::DepthCopy(resultJson);
        resultJson.Clear();
        EXPECT_EQ(newJson["content"].AsString(), content);
    }

    TEST(JsonReaderTest, ObjectReplaceTest)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        objVal.Replace("name", g_newName.c_str());
        EXPECT_EQ(objVal["name"].AsString(), g_newName);
        objVal.Replace("isChild", g_newIsChild);
        EXPECT_EQ(objVal["isChild"].AsBool(), g_newIsChild);
        objVal.Replace("age", g_newAge);
        EXPECT_EQ(objVal["age"].AsInt(), g_newAge);
        objVal.Replace("code1", g_newCode1);
        EXPECT_EQ(objVal["code1"].AsUInt(), g_newCode1);
        objVal.Replace("code2", g_newCode2);
        EXPECT_EQ(objVal["code2"].AsInt64(), g_newCode2);
        objVal.Replace("height", g_newHeight);
        EXPECT_EQ(objVal["height"].AsDouble(), g_newHeight);
        std::string content = "content";
        Json2::Value resultJson = JsonReader::CreateObject();
        resultJson.Add("content", JsonReader::CreateString(content));
        objVal.Replace("school", resultJson);
        EXPECT_EQ(objVal["school"]["content"].AsString(), content);
    }

    TEST(JsonReaderTest, ArrayReplaceTest)
    {
        Json2::Value arrVal = JsonReader::ParseJsonData2(g_arr);
        int index = 0;
        arrVal.Replace(index, g_newName.c_str());
        EXPECT_EQ(arrVal.GetArrayItem(index++).AsString(), g_newName);
        arrVal.Replace(index, g_newIsChild);
        EXPECT_EQ(arrVal.GetArrayItem(index++).AsBool(), g_newIsChild);
        arrVal.Replace(index, g_newAge);
        EXPECT_EQ(arrVal.GetArrayItem(index++).AsInt(), g_newAge);
        arrVal.Replace(index, g_newCode1);
        EXPECT_EQ(arrVal.GetArrayItem(index++).AsUInt(), g_newCode1);
        arrVal.Replace(index, g_newCode2);
        EXPECT_EQ(arrVal.GetArrayItem(index++).AsInt64(), g_newCode2);
        arrVal.Replace(index, g_newHeight);
        EXPECT_EQ(arrVal.GetArrayItem(index++).AsDouble(), g_newHeight);
        std::string content = "content";
        Json2::Value resultJson = JsonReader::CreateObject();
        resultJson.Add("content", JsonReader::CreateString(content));
        arrVal.Replace(index, resultJson);
        EXPECT_EQ(arrVal.GetArrayItem(index)["content"].AsString(), content);
    }

    TEST(JsonReaderTest, IsValueTest)
    {
        Json2::Value arrVal = JsonReader::ParseJsonData2(g_arr);
        EXPECT_TRUE(arrVal.IsArray());
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        EXPECT_FALSE(objVal.IsNull());
        EXPECT_TRUE(objVal.IsMember("school"));
        EXPECT_TRUE(objVal["name"].IsString());
        EXPECT_TRUE(objVal["age"].IsNumber());
        EXPECT_TRUE(objVal["age"].IsInt());
        EXPECT_TRUE(objVal["code1"].IsUInt());
        EXPECT_TRUE(objVal["code2"].IsUInt64());
        EXPECT_TRUE(objVal["isChild"].IsBool());
        EXPECT_TRUE(objVal["height"].IsDouble());
    }

    TEST(JsonReaderTest, GetValueTest)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        EXPECT_EQ(objVal.GetString("name"), g_name);
        EXPECT_EQ(objVal.GetBool("isChild"), g_isChild);
        EXPECT_EQ(objVal.GetInt("age"), g_age);
        EXPECT_EQ(objVal.GetUInt("code1"), g_code1);
        EXPECT_EQ(objVal.GetInt64("code2"), g_code2);
        EXPECT_EQ(objVal.GetFloat("height"), (float)g_height);
        EXPECT_EQ(objVal.GetDouble("height"), g_height);
        EXPECT_EQ(objVal.GetDouble("height1", 0.0), 0.0); // key not exist retrun default value
        EXPECT_EQ(objVal.GetValue("school").GetString("schoolName"), g_schoolName);
        EXPECT_EQ(objVal.GetValue("school").GetString("schoolAddr"), g_schoolAddr);
    }

    TEST(JsonReaderTest, OperatorBracketCharPtrTest)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        const char* key = "name";
        Json2::Value name = objVal[key];
        std::string value = name.AsString();
        EXPECT_EQ(value, g_name);
        // key not exist
        const char* key2 = "name22";
        Json2::Value name2 = objVal[key2];
        std::string value2 = name2.AsString();
        EXPECT_EQ(value2, "");
    }

    TEST(JsonReaderTest, OperatorBracketConstCharPtrTest)
    {
        const Json2::Value objVal = static_cast<const Json2::Value>(JsonReader::ParseJsonData2(g_obj));
        const char* key = "name";
        const Json2::Value name = objVal[key];
        std::string value = name.AsString();
        EXPECT_EQ(value, g_name);
        // key not exist
        const char* key2 = "name22";
        Json2::Value name2 = objVal[key2];
        std::string value2 = name2.AsString();
        EXPECT_EQ(value2, "");
    }

    TEST(ValueTest, OperatorBracketStdStringTest)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        std::string key = "name";
        Json2::Value name = objVal[key];
        std::string value = name.AsString();
        EXPECT_EQ(value, g_name);
        // key not exist
        std::string key2 = "name22";
        Json2::Value name2 = objVal[key2];
        std::string value2 = name2.AsString();
        EXPECT_EQ(value2, "");
    }

    TEST(ValueTest, OperatorBracketConstStdStringTest)
    {
        const Json2::Value objVal = static_cast<const Json2::Value>(JsonReader::ParseJsonData2(g_obj));
        std::string key = "name";
        const Json2::Value name = objVal[key];
        std::string value = name.AsString();
        EXPECT_EQ(value, g_name);
        // key not exist
        std::string key2 = "name22";
        Json2::Value name2 = objVal[key2];
        std::string value2 = name2.AsString();
        EXPECT_EQ(value2, "");
    }

    TEST(ValueTest, GetMemberNamesTest)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        Json2::Value::Members members = objVal.GetMemberNames();
        int size = 7;
        int memberSize = members.size();
        EXPECT_EQ(memberSize, size);
    }

    TEST(ValueTest, ToStringTest_NoJsonPtr)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        std::string key2 = "name22";
        Json2::Value name2 = objVal[key2];
        std::string str = name2.ToString();
        EXPECT_TRUE(str.empty());
    }

    TEST(ValueTest, ToStyledStringTest)
    {
        // no jsonPtr
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        std::string key1 = "name22";
        Json2::Value name1= objVal[key1];
        std::string str1 = name1.ToStyledString();
        EXPECT_TRUE(str1.empty());
        // jsonPtr exist
        std::string key2 = "name";
        Json2::Value name2= objVal[key2];
        std::string str2 = name2.ToStyledString();
        EXPECT_FALSE(str2.empty());
    }

    TEST(ValueTest, IsIntTest)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        std::string key1 = "name";
        Json2::Value name = objVal[key1];
        EXPECT_FALSE(name.IsInt());

        std::string key2 = "age";
        Json2::Value age = objVal[key2];
        EXPECT_TRUE(age.IsInt());
    }

    TEST(ValueTest, IsUIntTest)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        std::string key1 = "name";
        Json2::Value name = objVal[key1];
        EXPECT_FALSE(name.IsUInt());

        std::string key2 = "age";
        Json2::Value age = objVal[key2];
        EXPECT_TRUE(age.IsUInt());
    }

    TEST(ValueTest, IsInt64Test)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        std::string key1 = "name";
        Json2::Value name = objVal[key1];
        EXPECT_FALSE(name.IsInt64());

        std::string key2 = "age";
        Json2::Value age = objVal[key2];
        EXPECT_TRUE(age.IsInt64());
    }

    TEST(ValueTest, IsUInt64Test)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        std::string key1 = "name";
        Json2::Value name = objVal[key1];
        EXPECT_FALSE(name.IsUInt64());

        std::string key2 = "age";
        Json2::Value age = objVal[key2];
        EXPECT_TRUE(age.IsUInt64());
    }

    TEST(ValueTest, IsDoubleTest)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        std::string key1 = "name";
        Json2::Value name = objVal[key1];
        EXPECT_FALSE(name.IsDouble());

        std::string key2 = "age";
        Json2::Value age = objVal[key2];
        EXPECT_TRUE(age.IsDouble());
    }

    TEST(ValueTest, IsObjectTest)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        std::string key = "school";
        Json2::Value school = objVal[key];
        EXPECT_TRUE(school.IsObject());
    }

    TEST(ValueTest, GetBoolTest_TypeErr)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        bool ret = objVal.GetBool("name");
        EXPECT_FALSE(ret);
    }

    TEST(ValueTest, GetStringTest_TypeErr)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        std::string ret = objVal.GetString("age");
        EXPECT_EQ(ret, "");
    }

    TEST(ValueTest, AsFolatTest_TypeErr)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        Json2::Value height = objVal.GetValue("height");
        float val = height.AsFloat();
        float expectVal = 165.3;
        EXPECT_EQ(val, expectVal);
    }

    TEST(ValueTest, AsDoubleTest_TypeErr)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        Json2::Value name = objVal.GetValue("name");
        double val = name.AsDouble();
        float expectVal = 0.0;
        EXPECT_EQ(val, expectVal);
    }

    TEST(ValueTest, AsBoolTest_TypeErr)
    {
        Json2::Value objVal = JsonReader::ParseJsonData2(g_obj);
        Json2::Value name = objVal.GetValue("name");
        bool val = name.AsBool();
        EXPECT_FALSE(val);
    }

    TEST(JsonReaderTest, CreateNullTest)
    {
        Json2::Value value = JsonReader::CreateNull();
        EXPECT_TRUE(value.IsNull());
    }

    TEST(JsonReaderTest, GetKeyTest)
    {
        Json2::Value jsonData1 = JsonReader::ParseJsonData2(g_obj);
        Json2::Value height = jsonData1.GetValue("height");
        std::string str = height.GetKey();
        EXPECT_EQ(str, "height");
    }

    TEST(JsonReaderTest, ReadFileTest)
    {
        std::string path = "1.json";
        std::ofstream file(path);
        if (!file) {
            printf("Error creating file: %s\r\n", path.c_str());
            EXPECT_TRUE(false);
            return;
        }
        file << g_obj;
        file.close();
        std::string readStr = JsonReader::ReadFile(path);
        EXPECT_EQ(readStr, g_obj);
    }

    TEST(JsonReaderTest, GetErrorPtrTest)
    {
        Json2::Value jsonData = JsonReader::ParseJsonData2(g_obj);
        std::string errors = JsonReader::GetErrorPtr();
        EXPECT_FALSE(jsonData.IsNull());
        EXPECT_TRUE(errors.empty());
    }

    TEST(JsonReaderTest, AddTest_Err)
    {
        Json2::Value jsonData = JsonReader::ParseJsonData2(g_obj);
        // key is nullptr
        const char* key = nullptr;
        bool ret = jsonData.Add(key, "aaa");
        EXPECT_FALSE(ret);
        ret = jsonData.Add(key, false);
        EXPECT_FALSE(ret);
        double d = 22.0;
        ret = jsonData.Add(key, d);
        EXPECT_FALSE(ret);
        std::string str = "{\"name\":\"jin\", \"age\":20}";
        Json2::Value val = JsonReader::ParseJsonData2(str);
        ret = jsonData.Add(key, val);
        EXPECT_FALSE(ret);
    }

    TEST(JsonReaderTest, AddTest_ArrayErr)
    {
        Json2::Value jsonData = JsonReader::ParseJsonData2(g_obj);
        bool ret = jsonData.Add("aaa");
        EXPECT_FALSE(ret);
        ret = jsonData.Add(false);
        EXPECT_FALSE(ret);
        double d = 436.96;
        ret = jsonData.Add(d);
        EXPECT_FALSE(ret);
        Json2::Value val = JsonReader::CreateNull();
        ret = jsonData.Add(val);
        EXPECT_FALSE(ret);
    }

    TEST(JsonReaderTest, ReplaceTest_Err)
    {
        Json2::Value jsonData = JsonReader::ParseJsonData2(g_obj);
        // key is nullptr
        const char* key = nullptr;
        bool ret = jsonData.Replace(key, "aaa");
        EXPECT_FALSE(ret);
        ret = jsonData.Replace(key, false);
        EXPECT_FALSE(ret);
        double d = 22.0;
        ret = jsonData.Replace(key, d);
        EXPECT_FALSE(ret);
        std::string str = "{\"name\":\"jin\", \"age\":20}";
        Json2::Value val = JsonReader::ParseJsonData2(str);
        ret = jsonData.Replace(key, val);
        EXPECT_FALSE(ret);
    }

    TEST(JsonReaderTest, ReplaceTest_ArrayErr)
    {
        Json2::Value jsonData = JsonReader::ParseJsonData2(g_obj);
        // key is nullptr
        const char* key = nullptr;
        int invalidIndex = -1;
        bool ret = jsonData.Replace(invalidIndex, "aaa");
        EXPECT_FALSE(ret);
        ret = jsonData.Replace(invalidIndex, false);
        EXPECT_FALSE(ret);
        double d = 22.0;
        ret = jsonData.Replace(invalidIndex, d);
        EXPECT_FALSE(ret);
        std::string str = "{\"name\":\"jin\", \"age\":20}";
        Json2::Value val = JsonReader::ParseJsonData2(str);
        ret = jsonData.Replace(invalidIndex, val);
        EXPECT_FALSE(ret);
    }
}