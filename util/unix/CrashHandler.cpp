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

#include "CrashHandler.h"

#include <csignal>
#include <cstdlib>
#include <execinfo.h>
#include <sstream>
#include <unistd.h>

#include "PreviewerEngineLog.h"
#include "PublicMethods.h"

using namespace std;

void CrashHandler::InitExceptionHandler()
{
    if (signal(SIGSEGV, ApplicationCrashHandler) == SIG_ERR) {
        ELOG("InitExceptionHandler failed");
        return;
    }
}

void CrashHandler::ApplicationCrashHandler(int signal)
{
    const uint32_t MAX_STACK_SIZE = 128;
    int8_t crashBeginLog[] = "[JsEngine Crash]Engine Crash Info Begin.\n";
    write(STDERR_FILENO, crashBeginLog, sizeof(crashBeginLog) - 1);
    int8_t stackIntLog[PublicMethods::MAX_ITOA_BIT] = {0};
    uint32_t itoaLength = PublicMethods::Ulltoa(signal, stackIntLog);
    int8_t signalLog[] = "[JsEngine Crash]Error: signal : 0x";
    write(STDERR_FILENO, signalLog, sizeof(signalLog) - 1);
    write(STDERR_FILENO, stackIntLog, itoaLength);

    // get void*'s for all entries on the stack
    void *array[10];
    size_t size = backtrace(array, MAX_STACK_SIZE);
    // print out all the frames to stdout
    backtrace_symbols_fd(array, size, STDERR_FILENO);

    int8_t crashEndLog[] = "\n[JsEngine Crash]Engine Crash Info End.\n";
    write(STDERR_FILENO, crashEndLog, sizeof(crashEndLog) - 1);
}