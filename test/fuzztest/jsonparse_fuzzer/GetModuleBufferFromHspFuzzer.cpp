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
#include <map>
#include <fstream>
#include <iostream>
#include <gtest/gtest.h>
#include <filesystem>
#include "cJSON.h"
#include "secodeFuzz.h"
#include "common.h"
#include "ChangeJsonUtil.h"
#include "StageContext.h"
#include "MockFile.h"
using namespace testmock;
using namespace fuzztest;

namespace {
TEST(GetModuleBufferFromHspFuzzTest, test_json)
{
    std::cout << "--> GetModuleBufferFromHspFuzzTest for common start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"GetModuleBufferFromHspFuzzTest", 0)
    {
        // 变化数据
        const int defaultLength = 1000;
        const std::string defaultString = "aaaa";
        std::string strVal = DT_SetGetString(&g_Element[0], defaultString.size() + 1, defaultLength,
            (char*)defaultString.c_str());
        std::string newFileName = MockFile::CreateHspFile("testHspFile", strVal);
        // 执行接口
        OHOS::Ide::StageContext::GetInstance().GetModuleBufferFromHsp("aaa", "bbb");
        OHOS::Ide::StageContext::GetInstance().GetModuleBufferFromHsp(newFileName, "bbb.txt");
        std::vector<uint8_t>* buf =
            OHOS::Ide::StageContext::GetInstance().GetModuleBufferFromHsp(newFileName, "ets/modules.abc");
        if (buf) {
            delete buf;
        }
    }
    DT_FUZZ_END()
    printf("end ---- GetModuleBufferFromHspFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test GetModuleBufferFromHspFuzzTest is not ok\r\n");
    } else {
        printf("test GetModuleBufferFromHspFuzzTest is ok\r\n");
    }
    std::cout << "--> GetModuleBufferFromHspFuzzTest for common end <--" << std::endl;
}
}