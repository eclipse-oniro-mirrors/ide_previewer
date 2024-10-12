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
#include <filesystem>
#include "FileSystem.h"
#include "gtest/gtest.h"
#define private public
#include "StageContext.h"
#include "MockFile.h"
using namespace testmock;

namespace {
    class StageContextTest : public ::testing::Test {
    public:
        StageContextTest() {}
        ~StageContextTest() {}
    protected:
        static void WriteFile(const std::string& filePath, const std::string& content)
        {
            std::ofstream file(filePath, std::ios_base::trunc);
            if (file.is_open()) {
                file << content;
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
                WriteFile(testFile, testLoadJsonContent);

                testFileError = testDir + "/testfileError";
                WriteFile(testFileError, R"({"anBuildOutPut","anBuildMode":"type"})");

                testfileErrorMember = testDir + "/testfileErrorMember";
                WriteFile(testfileErrorMember, R"({"modulePathMap":{"entry":"entry","lib1":"lib1"},
                    "harNameOhmMap1":{"lib1":"lib1"}, "packageManagerType":"ohpm", "compileEntry":[],
                    "dynamicImportLibInfo":{}, "routerMap":[], "anBuildOutPut":"arm64-v8a", "anBuildMode":"type"})");
                std::string hspDir = testDir + "/MyApplication32/oh_modules/.hsp";
                if (!FileSystem::IsDirectoryExists(hspDir)) {
                    FileSystem::MakeDir(testDir + "/MyApplication32");
                    FileSystem::MakeDir(testDir + "/MyApplication32/oh_modules");
                    FileSystem::MakeDir(testDir + "/MyApplication32/oh_modules/.hsp");
                    FileSystem::MakeDir(testDir + "/MyApplication32/oh_modules/.hsp/example@1.0.0");

                    testFileHsp = hspDir + "/example.hsp";
                    WriteFile(testFileHsp, "example.hsp");
                }
                
                if (!FileSystem::IsDirectoryExists("ets")) {
                    FileSystem::MakeDir(testDir + "/ets");
                }
                WriteFile(testDir + "/ets/buildConfig.json", "{\"aceModuleBuild\":\"" + testDir + "/ets" + "\"}");
            } else {
                printf("error: getcwd failed\n");
            }
        }

        static void TearDownTestCase()
        {
            if (std::remove(testFile.c_str()) != 0) {
                printf("Error deleting file!\n");
            }

            // 最后删除ets文件夹
            char buffer[FILENAME_MAX];
            if (getcwd(buffer, FILENAME_MAX) != nullptr) {
                testDir = std::string(buffer);
                std::string etsDir = testDir + "/ets";
                if (FileSystem::IsDirectoryExists(etsDir)) {
                    std::filesystem::remove_all(etsDir);
                }
            }
        }
        static std::string testDir;
        static std::string testFile;
        static std::string testFileError;
        static std::string testfileErrorMember;
        static std::string testFileHsp;
        static std::string testLoadJsonContent;
    };
    std::string StageContextTest::testDir = "";
    std::string StageContextTest::testFile = "";
    std::string StageContextTest::testFileError = "";
    std::string StageContextTest::testFileHsp = "";
    std::string StageContextTest::testfileErrorMember = "";
    std::string StageContextTest::testLoadJsonContent =
        R"({"modulePathMap":{"entry":"entry","lib1":"lib1"}, "compileMode":"esmodule",
            "projectRootPath":"./MyApplication32", "nodeModulesPath":"node_modules", "moduleName":"entry",
            "harNameOhmMap":{"lib1":"lib1"}, "hspNameOhmMap":{"lib1":"lib1"}, "buildConfigPath":"buildConfig.json",
            "packageManagerType":"ohpm", "compileEntry":[], "dynamicImportLibInfo":{}, "routerMap":[],
            "anBuildOutPut":"arm64-v8a", "anBuildMode":"type"})";

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
        OHOS::Ide::StageContext::GetInstance().SetLoaderJsonPath(testFileError);
        OHOS::Ide::StageContext::GetInstance().GetModulePathMapFromLoaderJson();
        OHOS::Ide::StageContext::GetInstance().SetLoaderJsonPath(testfileErrorMember);
        OHOS::Ide::StageContext::GetInstance().GetModulePathMapFromLoaderJson();
        OHOS::Ide::StageContext::GetInstance().SetLoaderJsonPath(testFile);
        OHOS::Ide::StageContext::GetInstance().GetModulePathMapFromLoaderJson();
        EXPECT_EQ(OHOS::Ide::StageContext::GetInstance().modulePathMap.size(), 2);
        EXPECT_EQ(OHOS::Ide::StageContext::GetInstance().hspNameOhmMap.size(), 1);
        EXPECT_EQ(OHOS::Ide::StageContext::GetInstance().projectRootPath, "./MyApplication32");
    }

    TEST_F(StageContextTest, GetHspAceModuleBuildTest)
    {
        std::string fileContent = R"({"aceModuleBuild":"abc"})";
        WriteFile(testFile, fileContent);
        std::string ret = OHOS::Ide::StageContext::GetInstance().GetHspAceModuleBuild(testFile);
        EXPECT_EQ(ret, "abc");

        ret = OHOS::Ide::StageContext::GetInstance().GetHspAceModuleBuild("aaa");
        EXPECT_EQ(ret, "");

        fileContent = "";
        WriteFile(testFile, fileContent);
        ret = OHOS::Ide::StageContext::GetInstance().GetHspAceModuleBuild(testFile);
        EXPECT_EQ(ret, "");

        fileContent = R"({"aceModuleBuild11":"abc"})";
        WriteFile(testFile, fileContent);
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
        WriteFile(testFile, fileContent);
        retMap = OHOS::Ide::StageContext::GetInstance().ParseMockJsonFile(testFile);
        EXPECT_EQ(retMap.size(), 0);

        fileContent = R"({
            "libentry.so": {
                "source": 123
            }
        })";
        WriteFile(testFile, fileContent);
        retMap = OHOS::Ide::StageContext::GetInstance().ParseMockJsonFile(testFile);
        EXPECT_EQ(retMap.size(), 0);

        fileContent = R"({
            "libentry.so": {
                "source": "src/mock/Libentry.mock.ets"
            }
        })";
        WriteFile(testFile, fileContent);
        retMap = OHOS::Ide::StageContext::GetInstance().ParseMockJsonFile(testFile);
        EXPECT_EQ(retMap["libentry.so"], "src/mock/Libentry.mock.ets");
    }

    TEST_F(StageContextTest, GetModuleBufferTest)
    {
        std::vector<uint8_t>* ret =
            OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundleTestlibrary");
        EXPECT_TRUE(ret == nullptr);

        ret = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("/Testlibrary");
        EXPECT_TRUE(ret == nullptr);

        OHOS::Ide::StageContext::GetInstance().modulePathMap.clear();
        ret = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundle1/Testlibrary");
        EXPECT_TRUE(ret == nullptr);

        OHOS::Ide::StageContext::GetInstance().modulePathMap["Testlibrary"] = "TestlibraryPathX";
        ret = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundle/TestlibraryX");
        EXPECT_TRUE(ret == nullptr);

        OHOS::Ide::StageContext::GetInstance().modulePathMap["Testlibrary"] = "TestlibraryPath";
        ret = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundle/Testlibrary");
        EXPECT_TRUE(ret == nullptr);

        OHOS::Ide::StageContext::GetInstance().modulePathMap["Testlibrary"] = "/tmp/../";
        ret = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundle/Testlibrary");
        EXPECT_TRUE(ret == nullptr);

        OHOS::Ide::StageContext::GetInstance().modulePathMap["Testlibrary"] = "/tmp/../";
        OHOS::Ide::StageContext::GetInstance().hspNameOhmMap["TestlibraryM"] ="@bundle:aaa.bbb.ccc/TestlibraryM/Index";
        ret = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundle/TestlibraryM");
        EXPECT_TRUE(ret == nullptr);

        char buffer[FILENAME_MAX];
        if (getcwd(buffer, FILENAME_MAX) != nullptr) {
            testDir = std::string(buffer);
            std::string testlibrary = testDir + "/ets";
            OHOS::Ide::StageContext::GetInstance().modulePathMap["Testlibrary"] = testlibrary;
            ret = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundle/Testlibrary");
            EXPECT_TRUE(ret == nullptr);
            if (FileSystem::IsDirectoryExists(testlibrary)) {
                std::filesystem::remove(testlibrary + "/modules.abc");
                ret = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundle/Testlibrary");
                EXPECT_TRUE(ret == nullptr);

                const int defaultLength = 1000;
                const std::string defaultString = "aaaa";
                std::string strVal = "bbb";
                MockFile::CreateHspFile("testHspFile", strVal);

                std::vector<uint8_t>* ret2 =
                    OHOS::Ide::StageContext::GetInstance().GetModuleBuffer("bundle/Testlibrary");
                OHOS::Ide::StageContext::GetInstance().ReleaseHspBuffers();
                EXPECT_FALSE(ret2 == nullptr);
            }
        }
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

    TEST_F(StageContextTest, GetActualCloudHspDirTest)
    {
        std::string ret = OHOS::Ide::StageContext::GetInstance().GetActualCloudHspDir("ddd");
        EXPECT_TRUE(ret == "");
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

        ret = OHOS::Ide::StageContext::GetInstance().CompareHspVersion("10.1", "10.1.2");
        EXPECT_EQ(ret, -1);

        ret = OHOS::Ide::StageContext::GetInstance().CompareHspVersion("10.1.2.3", "10.1.2");
        EXPECT_EQ(ret, 1);
    }

    TEST_F(StageContextTest, GetUpwardDirIndexTest)
    {
        std::string path = "/aaa/bbb/ccc/ddd/eee/fff.json";
        int pos = OHOS::Ide::StageContext::GetInstance().GetUpwardDirIndex(path, 3);
        EXPECT_EQ(pos, 8); // 8 is expect pos value

        std::string path0 = "fff.json";
        int pos0 = OHOS::Ide::StageContext::GetInstance().GetUpwardDirIndex(path0, 3);
        EXPECT_EQ(pos0, -1);
    }

    TEST_F(StageContextTest, GetCloudModuleBufferTest)
    {
        std::vector<uint8_t>* ret = OHOS::Ide::StageContext::GetInstance().GetCloudModuleBuffer("aa");
        EXPECT_TRUE(ret == nullptr);

        char buffer[FILENAME_MAX];
        if (getcwd(buffer, FILENAME_MAX) != nullptr) {
            testDir = std::string(buffer);
            std::string hspDir = testDir + "/cloudHspDir";
            if (!FileSystem::IsDirectoryExists(hspDir)) {
                FileSystem::MakeDir(testDir + "/cloudHspDir");
                FileSystem::MakeDir(testDir + "/cloudHspDir/a");
                FileSystem::MakeDir(testDir + "/cloudHspDir/a/b");
                FileSystem::MakeDir(testDir + "/cloudHspDir/a/b/c");
                FileSystem::MakeDir(testDir + "/cloudHspDir/a/b/c/d");
                FileSystem::MakeDir(testDir + "/cloudHspDir/a/b/c/d/e");
                FileSystem::MakeDir(testDir + "/cloudHspDir/oh_modules");
                FileSystem::MakeDir(testDir + "/cloudHspDir/oh_modules/.hsp");
                FileSystem::MakeDir(testDir + "/cloudHspDir/oh_modules/.hsp/example@1.0.0");
            }
            std::string loderJsonPath = testDir + "/cloudHspDir/a/b/c/d/e/f.json";
            WriteFile(loderJsonPath, "test");
            std::string cloudHspPath = testDir + "/cloudHspDir/oh_modules/.hsp/example@1.0.0/example.hsp";
            WriteFile(cloudHspPath, "cloudHsp");
            OHOS::Ide::StageContext::GetInstance().SetLoaderJsonPath(loderJsonPath);

            OHOS::Ide::StageContext::GetInstance().hspNameOhmMap["example"] ="@bundle:aaa.bbb.ccc/example/Index";
            std::vector<uint8_t>* ret1 = OHOS::Ide::StageContext::GetInstance().GetCloudModuleBuffer("example");
            EXPECT_TRUE(ret1 == nullptr);
        }
    }

    TEST_F(StageContextTest, GetModuleBufferFromHspTest)
    {
        std::string path = "/aaa/bbb/ccc/ddd/eee/fff.json";
        std::vector<uint8_t>* ret = OHOS::Ide::StageContext::GetInstance().GetModuleBufferFromHsp(path, "aa");
        EXPECT_TRUE(ret == nullptr);
        std::string newFileName = MockFile::CreateHspFile("testHspFile", "abc");
        char buffer[FILENAME_MAX];
        if (getcwd(buffer, FILENAME_MAX) != nullptr) {
            testDir = std::string(buffer);
            std::string hspDir = testDir + "/testHspFile.hsp";
            std::vector<uint8_t>* buf1 =
                OHOS::Ide::StageContext::GetInstance().GetModuleBufferFromHsp(hspDir, "ets/modules.abc");
            OHOS::Ide::StageContext::GetInstance().ReleaseHspBuffers();
            EXPECT_FALSE(buf1 == nullptr);
        }
    }
    
    TEST_F(StageContextTest, GetSystemModuleBufferTest)
    {
        std::string path = "/aaa/bbb/ccc/ddd/eee/fff.json";
        std::vector<uint8_t>* ret = OHOS::Ide::StageContext::GetInstance().GetSystemModuleBuffer(path, "aa");
        EXPECT_TRUE(ret == nullptr);

        char buffer[FILENAME_MAX];
        if (getcwd(buffer, FILENAME_MAX) != nullptr) {
            testDir = std::string(buffer);
            std::string packageName = "com.huawei.example";
            std::string path1 = "bundle/" + packageName + "/moduleName";
            if (!FileSystem::IsDirectoryExists("/systemHsp")) {
                FileSystem::MakeDir(testDir + "/systemHsp");
                FileSystem::MakeDir(testDir + "/systemHsp/" + packageName);
                FileSystem::MakeDir(testDir + "/systemHsp/" + packageName + "/files");
            }
            std::string loderHspPath = testDir + "/systemHsp/example.hsp";
            WriteFile(loderHspPath, "test");

            OHOS::Ide::StageContext::GetInstance().SetHosSdkPath(testDir);
            std::vector<uint8_t>* ret1 = OHOS::Ide::StageContext::GetInstance().GetSystemModuleBuffer(path1, "example");
            EXPECT_TRUE(ret1 == nullptr);
        }
    }
}
