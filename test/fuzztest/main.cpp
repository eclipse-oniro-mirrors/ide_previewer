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

#include <gtest/gtest.h>
#include "secodeFuzz.h"
#include <iostream>
#include <common.h>

using namespace std;

const char* __asan_default_options()
{
    return "halt_on_error=0:log_path=./";
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    // 设置报告路径
    DT_Set_Report_Path(const_cast<char*>("./"));
    // 设置是否输出覆盖率sancov文件
    DT_Set_Is_Dump_Coverage(1);
    //设置用例单次执行多久超时，4.9版本后已经默认开启，默认600s
    DT_Set_TimeOut_Second(600);
    // 设置单个测试例运行时间，如果设置的运行次数没到而运行时间到了，提前结束
    DT_Set_Running_Time_Second(TIMEOUT_SECOND);
    // //关闭内存泄露检测
    DT_Enable_Leak_Check(0, 0);
    cout << DT_Get_Version() << endl;
    return RUN_ALL_TESTS();
}