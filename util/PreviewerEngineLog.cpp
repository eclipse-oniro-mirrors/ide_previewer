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

#include "PreviewerEngineLog.h"

#include <cstdio>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include "unistd.h"
#endif

#include "LocalDate.h"
#include "TimeTool.h"
#include "securec.h"

void PrintLog(const char* level, const char* file, const char* func, int line, const char* fmt, ...)
{
#ifdef NDEBUG
    std::string levelStr(level);
    if (levelStr == "DEBUG") {
        return;
    }
#endif
    static char output[1024] = { 0 };
    va_list argsList;
    va_start(argsList, fmt);
    std::string fileStr(file);
    int idx = fileStr.find_last_of("/");
    fileStr = fileStr.substr(idx + 1);
    int ret = vsnprintf_s(output, sizeof(output), sizeof(output), fmt, argsList);
    if (ret == -1) {
        std::cout << "PrintLog function error";
        return;
    }
    if (stdout != nullptr) {
        fprintf(stdout, "[%s][%s][%s][%d]%s:%s\n", level, fileStr.c_str(), func, line,
                TimeTool::GetFormatTime().c_str(), output);
        fflush(stdout);
    }
    va_end(argsList);
}