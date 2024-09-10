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
#define private public
#include "StageContext.h"

namespace {
    class StageContextTest : public ::testing::Test {
    public:
        StageContextTest() {}
        ~StageContextTest() {}
    protected:
        static void WriteFile(std::string fileContent)
        {
            std::ofstream file(testFile, std::ios_base::trunc);
            if (file.is_open()) {
                file << fileContent;
                file.close();
            } else {
                printf("Error open file!\n");
            }
        }

        static void SetUpTestCase()
        {
            char buffer[FILENAME_MAX];
            if (getcwd(buffer, FILENAME_MAX) != nullptr) {
                testDir = std::string(buffer);
                testFile = testDir + "/testfile";
                WriteFile(testLoadJsonContent);
            } else {
                printf("error: getcwd failed\n");
            }
        }

        static void TearDownTestCase()
        {
            if (std::remove(testFile.c_str()) != 0) {
                printf("Error deleting file!\n");
            }
        }
        static std::string testDir;
        static std::string testFile;
        static std::string testLoadJsonContent;
    };
    std::string StageContextTest::testDir = "";
    std::string StageContextTest::testFile = "";
    std::string StageContextTest::testLoadJsonContent =
        R"({"modulePathMap":{"entry":"entry","lib1":"lib1"},"compileMode":"esmodule",
            "projectRootPath":"MyApplication32", "nodeModulesPath":"node_modules","moduleName":"entry",
            "hspNameOhmMap":{"lib1":"lib1"},"packageManagerType":"ohpm", "compileEntry":[],
            "dynamicImportLibInfo":{}, "routerMap":[], "anBuildOutPut":"arm64-v8a", "anBuildMode":"type"})";

    TEST_F(StageContextTest, ReadFileContentsTest)
    {
        std::optional<std::vector<uint8_t>> buf1 =
            OHOS::Ide::StageContext::GetInstance().ReadFileContents("abc");
        EXPECT_TRUE(std::nullopt == buf1);
        std::optional<std::vector<uint8_t>> buf2 =
            OHOS::Ide::StageContext::GetInstance().ReadFileContents(testFile);
        EXPECT_TRUE(std::nullopt != buf2);
        if (buf2) {
            std::string str(buf2->begin(), buf2->end());
            EXPECT_EQ(str, testLoadJsonContent);
        } else {
            EXPECT_TRUE(false);
        }
    }

    TEST_F(StageContextTest, SetLoaderJsonPathTest)
    {
        OHOS::Ide::StageContext::GetInstance().SetLoaderJsonPath("aa");
        EXPECT_EQ(OHOS::Ide::StageContext::GetInstance().loaderJsonPath, "aa");
        OHOS::Ide::StageContext::GetInstance().SetLoaderJsonPath(testFile);
        EXPECT_EQ(OHOS::Ide::StageContext::GetInstance().loaderJsonPath, testFile);
    }

    TEST_F(StageContextTest, GetModulePathMapFromLoaderJsonTest)
    {
        OHOS::Ide::StageContext::GetInstance().SetLoaderJsonPath(testFile);
        OHOS::Ide::StageContext::GetInstance().GetModulePathMapFromLoaderJson();
        EXPECT_EQ(OHOS::Ide::StageContext::GetInstance().modulePathMap.size(), 2);
        EXPECT_EQ(OHOS::Ide::StageContext::GetInstance().hspNameOhmMap.size(), 1);
        EXPECT_EQ(OHOS::Ide::StageContext::GetInstance().projectRootPath, "MyApplication32");
    }

    TEST_F(StageContextTest, GetHspAceModuleBuildTest)
    {
        std::string fileContent = R"({"aceModuleBuild":"abc"})";
        WriteFile(fileContent);
        std::string ret = OHOS::Ide::StageContext::GetInstance().GetHspAceModuleBuild(testFile);
        EXPECT_EQ(ret, "abc");

        ret = OHOS::Ide::StageContext::GetInstance().GetHspAceModuleBuild("aaa");
        EXPECT_EQ(ret, "");

        fileContent = "";
        WriteFile(fileContent);
        ret = OHOS::Ide::StageContext::GetInstance().GetHspAceModuleBuild(testFile);
        EXPECT_EQ(ret, "");

        fileContent = R"({"aceModuleBuild11":"abc"})";
        WriteFile(fileContent);
        ret = OHOS::Ide::StageContext::GetInstance().GetHspAceModuleBuild(testFile);
        EXPECT_EQ(ret, "");
    }

    TEST_F(StageContextTest, ReleaseHspBuffersTest)
    {
        std::vector<uint8_t> *buf = new std::vector<uint8_t>();
        OHOS::Ide::StageContext::GetInstance().hspBufferPtrsVec.push_back(buf);
        EXPECT_TRUE(OHOS::Ide::StageContext::GetInstance().hspBufferPtrsVec.size() > 0);
        OHOS::Ide::StageContext::GetInstance().ReleaseHspBuffers();
        EXPECT_TRUE(OHOS::Ide::StageContext::GetInstance().hspBufferPtrsVec.size() == 0);
    }

    TEST_F(StageContextTest, ParseMockJsonFileTest)
    {
        std::map<std::string, std::string> retMap =
            OHOS::Ide::StageContext::GetInstance().ParseMockJsonFile("aaa");
        EXPECT_EQ(retMap.size(), 0);

        std::string fileContent = "";
        WriteFile(fileContent);
        retMap = OHOS::Ide::StageContext::GetInstance().ParseMockJsonFile(testFile);
        EXPECT_EQ(retMap.size(), 0);

        fileContent = R"({
            "libentry.so": {
                "source": 123
            }
        })";
        WriteFile(fileContent);
        retMap = OHOS::Ide::StageContext::GetInstance().ParseMockJsonFile(testFile);
        EXPECT_EQ(retMap.size(), 0);

        fileContent = R"({
            "libentry.so": {
                "source": "src/mock/Libentry.mock.ets"
            }
        })";
        WriteFile(fileContent);
        retMap = OHOS::Ide::StageContext::GetInstance().ParseMockJsonFile(testFile);
        EXPECT_EQ(retMap["libentry.so"], "src/mock/Libentry.mock.ets");
    }

    TEST_F(StageContextTest, GetModuleBufferTest)
    {
        std::vector<uint8_t>* ret =
            OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundleTestlibrary");
        EXPECT_TRUE(ret == nullptr);

        OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("/Testlibrary");
        EXPECT_TRUE(ret == nullptr);

        OHOS::Ide::StageContext::GetInstance().modulePathMap.clear();
        ret = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundle1/Testlibrary");
        EXPECT_TRUE(ret == nullptr);

        OHOS::Ide::StageContext::GetInstance().modulePathMap.clear();
        ret = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundle/Testlibrary");
        EXPECT_TRUE(ret == nullptr);

        OHOS::Ide::StageContext::GetInstance().modulePathMap["Testlibrary"] = "TestlibraryPath";
        ret = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundle/Testlibrary");
        EXPECT_TRUE(ret == nullptr);
    }

    TEST_F(StageContextTest, GetLocalModuleBufferTest)
    {
        OHOS::Ide::StageContext::GetInstance().modulePathMap["Testlibrary"] = "";
        std::vector<uint8_t>* ret =
            OHOS::Ide::StageContext::GetInstance().GetLocalModuleBuffer("Testlibrary");
        EXPECT_TRUE(ret == nullptr);

        OHOS::Ide::StageContext::GetInstance().modulePathMap["Testlibrary"] = ".";
        ret = OHOS::Ide::StageContext::GetInstance().GetLocalModuleBuffer("Testlibrary");
        EXPECT_TRUE(ret == nullptr);

        OHOS::Ide::StageContext::GetInstance().modulePathMap["Testlibrary"] = testDir;
        ret = OHOS::Ide::StageContext::GetInstance().GetLocalModuleBuffer("Testlibrary");
        EXPECT_TRUE(ret == nullptr);
    }

    TEST_F(StageContextTest, ContainsRelativePathTest)
    {
        bool ret = OHOS::Ide::StageContext::GetInstance().ContainsRelativePath("../../aaa");
        EXPECT_TRUE(ret);
        ret = OHOS::Ide::StageContext::GetInstance().ContainsRelativePath("bbb/aaa");
        EXPECT_FALSE(ret);
    }

    TEST_F(StageContextTest, GetModulePathMapTest)
    {
        OHOS::Ide::StageContext::GetInstance().modulePathMap["Testlibrary1"] = "aaa";
        EXPECT_EQ(OHOS::Ide::StageContext::GetInstance().GetModulePathMap()["Testlibrary1"], "aaa");
    }

    TEST_F(StageContextTest, GetCloudHspPathTest)
    {
        std::string ret = OHOS::Ide::StageContext::GetInstance().GetCloudHspPath("/aaa/bbb", "ccc");
        EXPECT_EQ(ret, "");
    }

    TEST_F(StageContextTest, ReplaceLastStrTest)
    {
        std::string ret = OHOS::Ide::StageContext::GetInstance().ReplaceLastStr("abcabcabc", "abc", "123");
        EXPECT_EQ(ret, "abcabc123");
    }

    TEST_F(StageContextTest, GetHspActualNameTest)
    {
        OHOS::Ide::StageContext::GetInstance().hspNameOhmMap["testlibrary1"] =
            "@bundle:aaa.bbb.ccc/Testlibrary/Index";
        std::string input = "Testlibrary";
        std::string ret;
        int num = OHOS::Ide::StageContext::GetInstance().GetHspActualName(input, ret);
        EXPECT_EQ(num, 1);
        EXPECT_EQ(ret, "testlibrary1");
    }

    TEST_F(StageContextTest, GetCloudHspVersionTest)
    {
        std::string ret = OHOS::Ide::StageContext::GetInstance().GetCloudHspVersion("/aaa/bbb@/ccc", "ddd");
        EXPECT_EQ(ret, "");

        ret = OHOS::Ide::StageContext::GetInstance().GetCloudHspVersion("/aaa/bbb@/ccc", "bbb");
        EXPECT_EQ(ret, "/ccc");
    }

    TEST_F(StageContextTest, SplitHspVersionTest)
    {
        std::vector<int> vec = OHOS::Ide::StageContext::GetInstance().StageContext::SplitHspVersion("10.1.2");
        EXPECT_EQ(vec[0], 10);
        EXPECT_EQ(vec[1], 1);
        EXPECT_EQ(vec[2], 2);
    }

    TEST_F(StageContextTest, CompareHspVersionTest)
    {
        int ret = OHOS::Ide::StageContext::GetInstance().CompareHspVersion("10.1.2", "10.1.1");
        EXPECT_EQ(ret, 1);

        ret = OHOS::Ide::StageContext::GetInstance().CompareHspVersion("10.1.2", "10.1.3");
        EXPECT_EQ(ret, -1);

        ret = OHOS::Ide::StageContext::GetInstance().CompareHspVersion("10.1.2", "10.1.2");
        EXPECT_EQ(ret, 0);
    }

    TEST_F(StageContextTest, GetUpwardDirIndexTest)
    {
        std::string path = "/aaa/bbb/ccc/ddd/eee/fff.json";
        int pos = OHOS::Ide::StageContext::GetInstance().GetUpwardDirIndex(path, 3);
        EXPECT_EQ(pos, 8); // 8 is expect pos value
    }

}