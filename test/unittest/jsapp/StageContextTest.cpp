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
#include "CommandParser.h"
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

        static void MakeDirs(const std::vector<std::string>& dirs)
        {
            for (const auto& dir : dirs) {
                if (!FileSystem::IsDirectoryExists(dir)) {
                    FileSystem::MakeDir(dir);
                }
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
            "anBuildOutPut":"arm64-v8a", "hspResourcesMap":{}, "anBuildMode":"type"})";

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

    TEST_F(StageContextTest, SetHosSdkPathTest)
    {
        OHOS::Ide::StageContext::GetInstance().SetHosSdkPath("/test/sdk/path");
        EXPECT_EQ(OHOS::Ide::StageContext::GetInstance().hosSdkPath, "/test/sdk/path");
        OHOS::Ide::StageContext::GetInstance().SetHosSdkPath("");
        EXPECT_EQ(OHOS::Ide::StageContext::GetInstance().hosSdkPath, "");
    }

    TEST_F(StageContextTest, GetHspActualNameMoreTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        std::map<std::string, std::string> backupMap = instance.hspNameOhmMap;
        instance.hspNameOhmMap.clear();
        // 无匹配
        std::string ret;
        int num = instance.GetHspActualName("nomatch", ret);
        EXPECT_EQ(num, 0);
        EXPECT_TRUE(ret.empty());
        // @normalized 标志分支
        instance.hspNameOhmMap["normlib"] = "@normalized:N&N&normlib&Index";
        ret.clear();
        num = instance.GetHspActualName("normlib", ret);
        EXPECT_EQ(num, 1);
        EXPECT_EQ(ret, "normlib");
        // 以组织名开头的包, '/' 替换为 '+'
        instance.hspNameOhmMap.clear();
        instance.hspNameOhmMap["@org/shared"] = "@bundle:aaa.bbb.ccc/shared/Index";
        ret.clear();
        num = instance.GetHspActualName("shared", ret);
        EXPECT_EQ(num, 1);
        EXPECT_EQ(ret, "@org+shared");
        // 多个同名匹配时取第一个
        instance.hspNameOhmMap["dup1"] = "@bundle:ddd.eee.fff/shared/Index";
        ret.clear();
        num = instance.GetHspActualName("shared", ret);
        EXPECT_EQ(num, 2);
        EXPECT_EQ(ret, "@org+shared");
        instance.hspNameOhmMap = backupMap;
    }

    TEST_F(StageContextTest, GetCloudProjectHspPathTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        std::string backupPath = instance.projectRootPath;
        // 项目根目录下不存在 oh_modules/.hsp
        instance.projectRootPath = "/not/exist/project";
        EXPECT_EQ(instance.GetCloudProjectHspPath("example"), "");
        // 返回以 actualName@ 开头的子目录
        instance.projectRootPath = testDir + "/MyApplication32";
        std::string ret = instance.GetCloudProjectHspPath("example");
        EXPECT_NE(ret.find("example@1.0.0"), std::string::npos);
        instance.projectRootPath = backupPath;
    }

    TEST_F(StageContextTest, GetCloudModuleHspPathTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        std::string backupPath = instance.loaderJsonPath;
        // 层级不足
        instance.loaderJsonPath = "f.json";
        EXPECT_EQ(instance.GetCloudModuleHspPath("example"), "");
        // 向上 5 级后存在 oh_modules/.hsp
        instance.loaderJsonPath = testDir + "/MyApplication32/a/b/c/d/e/f.json";
        std::string ret = instance.GetCloudModuleHspPath("example");
        EXPECT_NE(ret.find("example@1.0.0"), std::string::npos);
        instance.loaderJsonPath = backupPath;
    }

    TEST_F(StageContextTest, GetActualCloudHspDirMoreTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        std::string backupLoader = instance.loaderJsonPath;
        std::string backupRoot = instance.projectRootPath;
        std::string base = testDir + "/actualHspTest";
        MakeDirs({ base,
            base + "/proj", base + "/proj/oh_modules", base + "/proj/oh_modules/.hsp",
            base + "/proj/oh_modules/.hsp/lib@2.0.0",
            base + "/mod1", base + "/mod1/oh_modules", base + "/mod1/oh_modules/.hsp",
            base + "/mod1/oh_modules/.hsp/lib@1.0.0",
            base + "/mod1/a", base + "/mod1/b", base + "/mod1/c", base + "/mod1/d", base + "/mod1/e",
            base + "/mod2", base + "/mod2/oh_modules", base + "/mod2/oh_modules/.hsp",
            base + "/mod2/oh_modules/.hsp/lib@2.0.0",
            base + "/mod2/a", base + "/mod2/b", base + "/mod2/c", base + "/mod2/d", base + "/mod2/e",
            base + "/noMod", base + "/noMod/a", base + "/noMod/b", base + "/noMod/c",
            base + "/noMod/d", base + "/noMod/e" });
        // 模块级版本低, 加载项目级
        instance.loaderJsonPath = base + "/mod1/a/b/c/d/e/f.json";
        instance.projectRootPath = base + "/proj";
        std::string ret = instance.GetActualCloudHspDir("lib");
        EXPECT_NE(ret.find("proj/oh_modules/.hsp/lib@2.0.0"), std::string::npos);
        // 版本号相同, 优先加载模块级
        instance.loaderJsonPath = base + "/mod2/a/b/c/d/e/f.json";
        ret = instance.GetActualCloudHspDir("lib");
        EXPECT_NE(ret.find("mod2/oh_modules/.hsp/lib@2.0.0"), std::string::npos);
        // 项目级不存在, 加载模块级
        instance.projectRootPath = base + "/noProj";
        ret = instance.GetActualCloudHspDir("lib");
        EXPECT_NE(ret.find("mod2/oh_modules/.hsp/lib@2.0.0"), std::string::npos);
        // 模块级不存在, 加载项目级
        instance.loaderJsonPath = base + "/noMod/a/b/c/d/e/f.json";
        instance.projectRootPath = base + "/proj";
        ret = instance.GetActualCloudHspDir("lib");
        EXPECT_NE(ret.find("proj/oh_modules/.hsp/lib@2.0.0"), std::string::npos);
        // 恢复与清理
        instance.loaderJsonPath = backupLoader;
        instance.projectRootPath = backupRoot;
        std::filesystem::remove_all(base);
    }

    TEST_F(StageContextTest, UnzipHspFileTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        // 压缩包路径不存在
        std::vector<std::string> fileNames = { "module.json" };
        EXPECT_FALSE(instance.UnzipHspFile("/not/exist/file.hsp", testDir, fileNames));
        // 构造真实 hsp 压缩包
        std::string hspFile = MockFile::CreateHspFile("unzipTestHsp", "unzipContent");
        EXPECT_NE(hspFile, "");
        std::string writePath = testDir + "/unzipOut";
        MakeDirs({ writePath });
        // 压缩包内不存在的条目
        fileNames = { "notexist.txt" };
        EXPECT_FALSE(instance.UnzipHspFile(hspFile, writePath, fileNames));
        // 解压 module.json 成功
        fileNames = { "module.json" };
        EXPECT_TRUE(instance.UnzipHspFile(hspFile, writePath, fileNames));
        std::string outFilePath = writePath + FileSystem::GetSeparator() + "module.json";
        EXPECT_TRUE(FileSystem::IsFileExists(outFilePath));
        // 清理
        std::filesystem::remove_all(writePath);
        std::remove(hspFile.c_str());
        std::remove("module.json");
    }

    TEST_F(StageContextTest, GetModuleInfoTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        std::map<std::string, std::string> backupResources = instance.hspResourcesMap;
        std::map<std::string, std::string> backupOhmMap = instance.hspNameOhmMap;
        std::map<std::string, std::string> backupModulePath = instance.modulePathMap;
        std::string backupLoader = instance.loaderJsonPath;
        std::string backupRoot = instance.projectRootPath;
        std::vector<OHOS::Ide::HspInfo> dependencyHspInfos;
        // hspResourcesMap 为空
        instance.hspResourcesMap.clear();
        instance.GetModuleInfo(dependencyHspInfos);
        EXPECT_TRUE(dependencyHspInfos.empty());
        // hspNameOhmMap 无匹配, packageName 为空提前返回
        instance.hspResourcesMap["libx"] = "path/ResourceTable.txt";
        instance.hspNameOhmMap.clear();
        dependencyHspInfos.clear();
        instance.GetModuleInfo(dependencyHspInfos);
        EXPECT_TRUE(dependencyHspInfos.empty());
        // 云加载: hsp 不是合法 zip, 解压失败但 hspInfo 仍被收集
        std::string base = testDir + "/moduleInfoTest";
        std::string hspDir = base + "/oh_modules/.hsp/libx@1.0.0";
        MakeDirs({ base, base + "/oh_modules", base + "/oh_modules/.hsp", hspDir });
        WriteFile(hspDir + "/libx.hsp", "notzip");
        instance.loaderJsonPath = "f.json";
        instance.projectRootPath = base;
        instance.modulePathMap.clear();
        instance.hspNameOhmMap.clear();
        instance.hspNameOhmMap["libx"] = "@bundle:aaa.bbb.ccc/libx/Index";
        dependencyHspInfos.clear();
        instance.GetModuleInfo(dependencyHspInfos);
        EXPECT_EQ(dependencyHspInfos.size(), 1);
        EXPECT_EQ(dependencyHspInfos[0].moduleName, "libx");
        EXPECT_TRUE(dependencyHspInfos[0].resourcePath.empty());
        // 云加载成功: hsp 内含 module.json 与 resources.index
        WriteFile("module.json", R"({"module":{"name":"libx"}})");
        WriteFile("resources.index", "resources");
        EXPECT_TRUE(MockFile::CompressFiles({ "module.json", "resources.index" }, hspDir + "/libx.hsp"));
        dependencyHspInfos.clear();
        instance.GetModuleInfo(dependencyHspInfos);
        EXPECT_EQ(dependencyHspInfos.size(), 1);
        EXPECT_EQ(dependencyHspInfos[0].moduleName, "libx");
        EXPECT_NE(dependencyHspInfos[0].resourcePath.find("resources.index"), std::string::npos);
        EXPECT_FALSE(dependencyHspInfos[0].moduleJsonBuffer.empty());
        // 恢复与清理
        instance.hspResourcesMap = backupResources;
        instance.hspNameOhmMap = backupOhmMap;
        instance.modulePathMap = backupModulePath;
        instance.loaderJsonPath = backupLoader;
        instance.projectRootPath = backupRoot;
        std::remove("module.json");
        std::remove("resources.index");
        std::filesystem::remove_all(base);
    }

    TEST_F(StageContextTest, GetHspinfoTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        std::map<std::string, std::string> backupModulePath = instance.modulePathMap;
        std::map<std::string, std::string> backupResources = instance.hspResourcesMap;
        std::string backupRoot = instance.projectRootPath;
        std::string backupLoader = instance.loaderJsonPath;
        instance.hspResourcesMap.clear();
        instance.loaderJsonPath = "f.json";
        // modulePathMap 无该模块, 云加载失败
        instance.modulePathMap.clear();
        instance.projectRootPath = "/not/exist";
        OHOS::Ide::HspInfo hspInfo;
        hspInfo.moduleName = "liby";
        instance.GetHspinfo("liby", hspInfo);
        EXPECT_EQ(hspInfo.moduleName, "liby");
        EXPECT_TRUE(hspInfo.resourcePath.empty());
        EXPECT_TRUE(hspInfo.moduleJsonBuffer.empty());
        // modulePathMap 有该模块, 本地加载失败后回退云加载也失败
        instance.modulePathMap["liby"] = "exist";
        hspInfo = OHOS::Ide::HspInfo();
        hspInfo.moduleName = "liby";
        instance.GetHspinfo("liby", hspInfo);
        EXPECT_TRUE(hspInfo.resourcePath.empty());
        // 恢复
        instance.modulePathMap = backupModulePath;
        instance.hspResourcesMap = backupResources;
        instance.projectRootPath = backupRoot;
        instance.loaderJsonPath = backupLoader;
    }

    TEST_F(StageContextTest, GetLocalModuleInfoTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        std::map<std::string, std::string> backupResources = instance.hspResourcesMap;
        instance.hspResourcesMap.clear();
        OHOS::Ide::HspInfo hspInfo;
        hspInfo.moduleName = "libz";
        // hspResourcesMap 缺少该模块
        EXPECT_FALSE(instance.GetLocalModuleInfo(hspInfo));
        // modulePath 为空
        instance.hspResourcesMap["libz"] = "";
        EXPECT_FALSE(instance.GetLocalModuleInfo(hspInfo));
        // modulePath 为相对路径
        instance.hspResourcesMap["libz"] = "../libz/ResourceTable.txt";
        EXPECT_FALSE(instance.GetLocalModuleInfo(hspInfo));
        // modulePath 不含 ResourceTable.txt
        instance.hspResourcesMap["libz"] = "zzzz";
        EXPECT_FALSE(instance.GetLocalModuleInfo(hspInfo));
        // 成功: 目录下存在 module.json 与 resources.index
        std::string libDir = testDir + "/localModuleInfo";
        MakeDirs({ libDir });
        WriteFile(libDir + "/module.json", "{}");
        WriteFile(libDir + "/resources.index", "res");
        instance.hspResourcesMap["libz"] = libDir + "/ResourceTable.txt";
        EXPECT_TRUE(instance.GetLocalModuleInfo(hspInfo));
        EXPECT_EQ(hspInfo.resourcePath, libDir + "/resources.index");
        EXPECT_FALSE(hspInfo.moduleJsonBuffer.empty());
        // resources.index 不存在
        std::remove((libDir + "/resources.index").c_str());
        EXPECT_FALSE(instance.GetLocalModuleInfo(hspInfo));
        // module.json 不存在
        WriteFile(libDir + "/resources.index", "res");
        std::remove((libDir + "/module.json").c_str());
        EXPECT_FALSE(instance.GetLocalModuleInfo(hspInfo));
        // 恢复与清理
        instance.hspResourcesMap = backupResources;
        std::filesystem::remove_all(libDir);
    }

    TEST_F(StageContextTest, GetCloudModuleInfoTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        std::string backupRoot = instance.projectRootPath;
        std::string backupLoader = instance.loaderJsonPath;
        instance.loaderJsonPath = "f.json";
        instance.projectRootPath = "/not/exist";
        OHOS::Ide::HspInfo hspInfo;
        hspInfo.moduleName = "libc";
        // hspPath 不存在
        EXPECT_FALSE(instance.GetCloudModuleInfo("libc", hspInfo));
        // 构造 hsp 目录
        std::string base = testDir + "/cloudModuleInfo";
        std::string hspDir = base + "/oh_modules/.hsp/libc@1.0.0";
        MakeDirs({ base, base + "/oh_modules", base + "/oh_modules/.hsp", hspDir });
        instance.projectRootPath = base;
        // hsp 文件不是合法 zip
        WriteFile(hspDir + "/libc.hsp", "notzip");
        EXPECT_FALSE(instance.GetCloudModuleInfo("libc", hspInfo));
        // hsp 内缺少 resources.index 条目
        std::string hspFile = MockFile::CreateHspFile("cloudInfoHsp", "abcContent");
        EXPECT_NE(hspFile, "");
        std::filesystem::copy_file(hspFile, hspDir + "/libc.hsp",
            std::filesystem::copy_options::overwrite_existing);
        EXPECT_FALSE(instance.GetCloudModuleInfo("libc", hspInfo));
        // hsp 内含 module.json 与 resources.index, 加载成功
        WriteFile("module.json", "{}");
        WriteFile("resources.index", "res");
        EXPECT_TRUE(MockFile::CompressFiles({ "module.json", "resources.index" }, hspDir + "/libc.hsp"));
        hspInfo = OHOS::Ide::HspInfo();
        hspInfo.moduleName = "libc";
        EXPECT_TRUE(instance.GetCloudModuleInfo("libc", hspInfo));
        EXPECT_NE(hspInfo.resourcePath.find("resources.index"), std::string::npos);
        EXPECT_FALSE(hspInfo.moduleJsonBuffer.empty());
        // 恢复与清理
        instance.projectRootPath = backupRoot;
        instance.loaderJsonPath = backupLoader;
        std::remove(hspFile.c_str());
        std::remove("module.json");
        std::remove("resources.index");
        std::filesystem::remove_all(base);
    }

    TEST_F(StageContextTest, GetRouterMapTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        // 文件不存在
        EXPECT_TRUE(instance.GetRouterMap("/not/exist/router.json").empty());
        // 非法 json
        WriteFile(testFile, "notjson");
        EXPECT_TRUE(instance.GetRouterMap(testFile).empty());
        // 缺少 routerMap 节点
        WriteFile(testFile, R"({"aaa":"bbb"})");
        EXPECT_TRUE(instance.GetRouterMap(testFile).empty());
        // routerMap 不是数组
        WriteFile(testFile, R"({"routerMap":"notarray"})");
        EXPECT_TRUE(instance.GetRouterMap(testFile).empty());
        // 正常解析
        WriteFile(testFile, R"({
            "routerMap": [
                {"name":"page1", "pageSourceFile":"src/Page1.ets", "buildFunction":"Page1Builder",
                "ohmurl":"@bundle:aaa.bbb.ccc/entry/Page1", "bundleName":"aaa", "moduleName":"entry",
                "data":{"key1":"val1", "key2":123}},
                "invalidItem",
                {"name":"page2"}
            ]
        })");
        std::vector<OHOS::Ide::RouterItem> items = instance.GetRouterMap(testFile);
        EXPECT_EQ(items.size(), 2);
        EXPECT_EQ(items[0].name, "page1");
        EXPECT_EQ(items[0].pageSourceFile, "src/Page1.ets");
        EXPECT_EQ(items[0].buildFunction, "Page1Builder");
        EXPECT_EQ(items[0].ohmurl, "@bundle:aaa.bbb.ccc/entry/Page1");
        EXPECT_EQ(items[0].bundleName, "aaa");
        EXPECT_EQ(items[0].moduleName, "entry");
        EXPECT_EQ(items[0].data.size(), 1);
        EXPECT_EQ(items[0].data.at("key1"), "val1");
        EXPECT_EQ(items[1].name, "page2");
        EXPECT_TRUE(items[1].data.empty());
    }

    TEST_F(StageContextTest, SetPkgContextInfoTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        CommandParser& parser = CommandParser::GetInstance();
        std::string backupResourcePath = parser.appResourcePath;
        std::string backupLoaderJsonPath = parser.loaderJsonPath;
        const std::string moduleJsonPath = testDir + "/module.json";
        const std::string pkgContextInfoPath = testDir + "/pkgContextInfo.json";
        parser.appResourcePath = testDir;
        parser.loaderJsonPath = testDir + "/loader.json";
        std::map<std::string, std::string> pkgContextInfoJsonStringMap;
        std::map<std::string, std::string> packageNameList;
        instance.packageNameMap.clear();
        std::remove(moduleJsonPath.c_str());
        std::remove(pkgContextInfoPath.c_str());
        // module.json 不存在
        instance.SetPkgContextInfo(pkgContextInfoJsonStringMap, packageNameList);
        EXPECT_TRUE(pkgContextInfoJsonStringMap.empty());
        EXPECT_TRUE(packageNameList.empty());
        // module.json 无 module 节点
        WriteFile(moduleJsonPath, R"({"aaa":"bbb"})");
        instance.SetPkgContextInfo(pkgContextInfoJsonStringMap, packageNameList);
        EXPECT_TRUE(pkgContextInfoJsonStringMap.empty());
        EXPECT_TRUE(packageNameList.empty());
        // module.name 非字符串
        WriteFile(moduleJsonPath, R"({"module":{"name":333}})");
        instance.SetPkgContextInfo(pkgContextInfoJsonStringMap, packageNameList);
        EXPECT_TRUE(pkgContextInfoJsonStringMap.empty());
        EXPECT_TRUE(packageNameList.empty());
        // 有 name 无 packageName, pkgContextInfo.json 不存在
        WriteFile(moduleJsonPath, R"({"module":{"name":"entry"}})");
        instance.SetPkgContextInfo(pkgContextInfoJsonStringMap, packageNameList);
        EXPECT_TRUE(pkgContextInfoJsonStringMap.empty());
        EXPECT_TRUE(packageNameList.empty());
        // 有 packageName, pkgContextInfo.json 不存在
        WriteFile(moduleJsonPath, R"({"module":{"name":"entry","packageName":"entry"}})");
        instance.SetPkgContextInfo(pkgContextInfoJsonStringMap, packageNameList);
        EXPECT_EQ(packageNameList.size(), 1);
        EXPECT_EQ(packageNameList.at("entry"), "entry");
        EXPECT_TRUE(pkgContextInfoJsonStringMap.empty());
        // pkgContextInfo.json 存在, 解析填充 packageNameMap
        WriteFile(pkgContextInfoPath, R"({"entry":{"moduleName":"entryName"}})");
        pkgContextInfoJsonStringMap.clear();
        instance.SetPkgContextInfo(pkgContextInfoJsonStringMap, packageNameList);
        EXPECT_EQ(pkgContextInfoJsonStringMap.size(), 1);
        EXPECT_FALSE(pkgContextInfoJsonStringMap.at("entry").empty());
        EXPECT_EQ(instance.packageNameMap.size(), 1);
        EXPECT_EQ(instance.packageNameMap.at("entry"), "entryName");
        // loaderJsonPath 不含 loader.json 标志
        parser.loaderJsonPath = "/bbb/ccc/fff";
        pkgContextInfoJsonStringMap.clear();
        instance.SetPkgContextInfo(pkgContextInfoJsonStringMap, packageNameList);
        EXPECT_TRUE(pkgContextInfoJsonStringMap.empty());
        // 恢复与清理
        parser.appResourcePath = backupResourcePath;
        parser.loaderJsonPath = backupLoaderJsonPath;
        std::remove(moduleJsonPath.c_str());
        std::remove(pkgContextInfoPath.c_str());
    }

    TEST_F(StageContextTest, GetCloudModuleBufferMoreTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        std::map<std::string, std::string> backupOhmMap = instance.hspNameOhmMap;
        std::map<std::string, std::string> backupModulePath = instance.modulePathMap;
        std::string backupRoot = instance.projectRootPath;
        std::string backupLoader = instance.loaderJsonPath;
        // 同名 hsp 多个匹配时取第一个
        instance.hspNameOhmMap.clear();
        instance.hspNameOhmMap["aa"] = "@bundle:aaa.bbb.ccc/shared/Index";
        instance.hspNameOhmMap["bb"] = "@bundle:ddd.eee.fff/shared/Index";
        instance.modulePathMap.clear();
        instance.loaderJsonPath = "f.json";
        instance.projectRootPath = "/not/exist";
        EXPECT_TRUE(instance.GetCloudModuleBuffer("shared") == nullptr);
        // hsp 目录存在但 hsp 文件不存在
        std::string base = testDir + "/cloudBufTest";
        std::string hspDir = base + "/oh_modules/.hsp/aa@1.0.0";
        MakeDirs({ base, base + "/oh_modules", base + "/oh_modules/.hsp", hspDir });
        instance.projectRootPath = base;
        EXPECT_TRUE(instance.GetCloudModuleBuffer("shared") == nullptr);
        // hsp 文件存在, 解压获取 ets/modules.abc 成功
        std::string hspFile = MockFile::CreateHspFile("cloudBufHsp", "cloudabc");
        EXPECT_NE(hspFile, "");
        std::filesystem::copy_file(hspFile, hspDir + "/aa.hsp",
            std::filesystem::copy_options::overwrite_existing);
        std::vector<uint8_t>* buf = instance.GetCloudModuleBuffer("shared");
        EXPECT_FALSE(buf == nullptr);
        if (buf != nullptr) {
            std::string content(buf->begin(), buf->end());
            EXPECT_EQ(content, "cloudabc");
        }
        instance.ReleaseHspBuffers();
        // 恢复与清理
        instance.hspNameOhmMap = backupOhmMap;
        instance.modulePathMap = backupModulePath;
        instance.projectRootPath = backupRoot;
        instance.loaderJsonPath = backupLoader;
        std::remove(hspFile.c_str());
        std::remove("module.json");
        std::filesystem::remove_all(base);
    }

    TEST_F(StageContextTest, GetSystemModuleBufferMoreTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        EXPECT_TRUE(instance.GetSystemModuleBuffer("aaa/bbb/ccc", "moduleName") == nullptr);
        EXPECT_TRUE(instance.GetSystemModuleBuffer("bundle/com.test.example/other", "moduleName") == nullptr);
    }

    TEST_F(StageContextTest, GetModuleBufferLocalTest)
    {
        auto& instance = OHOS::Ide::StageContext::GetInstance();
        std::map<std::string, std::string> backupModulePath = instance.modulePathMap;
        std::string backupBuildConfig = instance.buildConfigPath;
        std::string libDir = testDir + "/localLib";
        MakeDirs({ libDir });
        WriteFile(libDir + "/modules.abc", "localabc");
        std::string buildConfigContent = R"({"aceModuleBuild":")" + libDir + R"("})";
        WriteFile(libDir + "/localBuildConfig.json", buildConfigContent);
        instance.modulePathMap["localLib"] = libDir;
        instance.buildConfigPath = "localBuildConfig.json";
        std::vector<uint8_t>* buf = instance.GetModuleBuffer("bundle/localLib");
        EXPECT_FALSE(buf == nullptr);
        if (buf != nullptr) {
            std::string content(buf->begin(), buf->end());
            EXPECT_EQ(content, "localabc");
        }
        instance.ReleaseHspBuffers();
        // 恢复与清理
        instance.modulePathMap = backupModulePath;
        instance.buildConfigPath = backupBuildConfig;
        std::filesystem::remove_all(libDir);
    }
}
