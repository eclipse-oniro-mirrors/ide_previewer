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

#ifndef PARAMS_PARSE_FUZZER_H
#define PARAMS_PARSE_FUZZER_H

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>

namespace fuzztest {
    static std::string g_currDir = "";
    static std::string g_currFile = "";

    class Param {
    public:
        Param(std::string name, std::vector<std::string> values) : name(name), values(values) {}
        std::string name;
        std::vector<std::string> values;
    };

    class ParamsParse {
    public:
        void ParamsParseFuzzTest();
    private:
        void CallParamsParseFunc(const std::vector<std::string>& args);
        void SetTestArgs(std::vector<std::string>& args);
        std::vector<Param> paramList = {
            Param("-j", { g_currDir }),
            Param("-n", { "entry" }),
            Param("-d", { "" }),
            Param("-p", { "8888" }),
            Param("-s", { "phone_1676450550023_1" }),
            Param("-or", { "1080", "2340" }),
            Param("-cr", { "1080", "2340" }),
            Param("-f", { g_currFile }),
            Param("-hs", { "102400" }),
            Param("-hf", { "true" }),
            Param("-shape", { "rect" }),
            Param("-device", { "phone" }),
            Param("-url", { "pages/Index" }),
            Param("-refresh", { "region" }),
            Param("-card", { "true" }),
            Param("-projectID", { "985150866" }),
            Param("-ts", { "trace_93488_commandPipe" }),
            Param("-cm", { "light" }),
            Param("-o", { "portrait" }),
            Param("-lws", { "40000" }),
            Param("-av", { "ACE_2_0" }),
            Param("-l", { "zh_CN" }),
            Param("-sd", { "480" }),
            Param("-sm", { "dynamic" }),
            Param("-arp", { g_currDir }),
            Param("-pm", { "Stage" }),
            Param("-pages", { "main_pages" }),
            Param("-hsp", { g_currDir }),
            Param("-cpm", { "true" }),
            Param("-abp", { "ets/entryability/EntryAbility.abc" }),
            Param("-abn", { "EntryAbility" }),
            Param("-staticCard", { "true" }),
            Param("-foldable", { "true" }),
            Param("-foldStatus", { "unfold" }),
            Param("-fr", { "1080", "2340" }),
            Param("-ljPath", { g_currFile })
        };
    };

    class ParamsParseFuzzTest : public testing::Test {
    protected:
        // 在整个测试夹具类执行前执行一次初始化操作
        static void SetUpTestCase()
        {
            // 使用空格分割字符串，并存入vector
            char buffer[FILENAME_MAX];
            if (getcwd(buffer, FILENAME_MAX) != nullptr) {
                g_currDir = std::string(buffer);
                g_currDir += "/MyApplication";
                int status = mkdir(g_currDir.c_str(), 0777); // 0777 表示所有用户有读、写、执行权限
                if (status != 0) {
                    printf("Error creating folder!\n");
                }
                g_currFile = g_currDir + "/test.json";
                // 创建文件流对象并打开文件
                std::ofstream file(g_currFile);
                // 检查文件是否成功打开
                if (file.is_open()) {
                    file.close();
                } else {
                    printf("Error creating file!\n");
                }
            } else {
                printf("error: getcwd failed\n");
            }
        }

        // 在整个测试夹具类执行后执行一次清理操作
        static void TearDownTestCase()
        {
            if (std::remove(g_currFile.c_str()) != 0) {
                printf("Error deleting file!\n");
            }
            std::filesystem::remove(g_currDir.c_str());
        }

        virtual void SetUp()
        {
            std::cout << "--> ParamsParseFuzzTest." << __func__ << " <--" <<std::endl;
        }

        virtual void TearDown()
        {
            std::cout << "--> ParamsParseFuzzTest." << __func__ << " <--" <<std::endl;
        }

        ParamsParse parse;
    };
}

#endif