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

#include <filesystem>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "gtest/gtest.h"
#include "NativeFileSystem.h"
using namespace std;

namespace {
    class NativeFileSystemTest : public ::testing::Test {
    public:
        NativeFileSystemTest() {}
        ~NativeFileSystemTest() {}
    protected:
        // 在整个测试夹具类执行前执行一次初始化操作
        static void SetUpTestCase()
        {
            char buffer[FILENAME_MAX];
            if (getcwd(buffer, FILENAME_MAX) != nullptr) {
                currDir = std::string(buffer);
                childDir = currDir + "/mytestdirname";
                int status = mkdir(childDir.c_str(), 0777); // 0777 表示所有用户有读、写、执行权限
                if (status != 0) {
                    printf("Error creating folder!\n");
                }
            } else {
                printf("error: getcwd failed");
            }
        }
        // 在整个测试夹具类执行后执行一次清理操作
        static void TearDownTestCase()
        {
            std::filesystem::remove(childDir.c_str());
        }

        static std::string currDir;
        static std::string childDir;
    };

    std::string NativeFileSystemTest::currDir = "";
    std::string NativeFileSystemTest::childDir = "";

    TEST_F(NativeFileSystemTest, FindSubfolderByNameTest)
    {
        std::string filePath = OHOS::Ide::NativeFileSystem::FindSubfolderByName(currDir, "mytestdirname");
        EXPECT_EQ(filePath, childDir);
    }
}