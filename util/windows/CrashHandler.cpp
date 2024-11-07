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
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <dbghelp.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <windows.h>

#include "PreviewerEngineLog.h"
#include "PublicMethods.h"

static const int MAX_NAME_LENGTH = 512;
static const int MAX_ADDRESS_LENGTH = 16;

void GetBacktrace(HANDLE hProcess, STACKFRAME64 &sf, SYMBOL_INFO *symbol)
{
    // get addr info
    if (SymFromAddr(hProcess, sf.AddrPC.Offset, 0, symbol)) {
        // get module info
        IMAGEHLP_MODULE64 modInfo;
        modInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
        if (!SymGetModuleInfo64(hProcess, sf.AddrPC.Offset, &modInfo)) {
            std::cout << "Unable to retrieve module information." << std::endl;
        }
        // print module name，function name and dll path
        std::cout << "0x" << std::setw(MAX_ADDRESS_LENGTH) << std::setfill('0') << std::hex << sf.AddrPC.Offset
            << std::dec << " in " << modInfo.ModuleName << "!_" << symbol->Name << " ()" << std::endl;
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD displacement = 0;
        // get line of source code
        if (SymGetLineFromAddr64(hProcess, sf.AddrPC.Offset, &displacement, &line)) {
            std::cout << "Source: " << line.FileName << " Line: " << line.LineNumber << std::endl;
        }
    } else {
        std::cout << "Unable to retrieve symbol for address 0x"
            << std::hex << sf.AddrPC.Offset << std::dec << std::endl;
    }
}

void CrashHandler::RecordCallStack(const CONTEXT *context)
{
    HANDLE hProcess = GetCurrentProcess();
    SymInitialize(hProcess, NULL, TRUE);
    CONTEXT crashContext = *context;
    STACKFRAME64 sf = {};
    DWORD imageType = IMAGE_FILE_MACHINE_I386;
#ifdef _M_X64
    imageType = IMAGE_FILE_MACHINE_AMD64;
    sf.AddrPC.Offset = crashContext.Rip;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrFrame.Offset = crashContext.Rsp;
    sf.AddrFrame.Mode = AddrModeFlat;
    sf.AddrStack.Offset = crashContext.Rsp;
    sf.AddrStack.Mode = AddrModeFlat;
#endif
    HANDLE hThread = GetCurrentThread();
    std::vector<BYTE> symbolBuffer(sizeof(SYMBOL_INFO) + MAX_NAME_LENGTH * sizeof(TCHAR));
    SYMBOL_INFO *symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer.data());
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_NAME_LENGTH;
    while (StackWalk64(imageType, hProcess, hThread, &sf, &crashContext,
                       NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
        if (sf.AddrPC.Offset == 0) {
            break;
        }
        GetBacktrace(hProcess, sf, symbol);
    }
    SymCleanup(hProcess);
}

LONG CrashHandler::ApplicationCrashHandler(EXCEPTION_POINTERS *exception)
{
    int8_t crashBeginLog[] = "[JsEngine Crash]Engine Crash Info Begin.\n";
    write(STDERR_FILENO, crashBeginLog, sizeof(crashBeginLog) - 1);

    int8_t stackIntLog[PublicMethods::MAX_ITOA_BIT] = {0};
    // 16 means hexadecimal
    unsigned int itoaLength = PublicMethods::Ulltoa(reinterpret_cast<uintptr_t>
                                                    (exception->ExceptionRecord->ExceptionAddress), stackIntLog);
    int8_t ELOG[] = "[JsEngine Crash]Address: 0x";
    write(STDERR_FILENO, ELOG, sizeof(ELOG) - 1);
    write(STDERR_FILENO, stackIntLog, itoaLength);
    // 16 means hexadecimal
    itoaLength = PublicMethods::Ulltoa(exception->ExceptionRecord->ExceptionCode, stackIntLog);
    int8_t addressLog[] = "\n[JsEngine Crash]ErrorCode: 0x";
    write(STDERR_FILENO, addressLog, sizeof(addressLog) - 1);
    write(STDERR_FILENO, stackIntLog, itoaLength);
    // 16 means hexadecimal
    RecordCallStack(exception->ContextRecord);

    int8_t crashEndLog[] = "\n[JsEngine Crash]Engine Crash Info End.\n";
    write(STDERR_FILENO, crashEndLog, sizeof(crashEndLog) - 1);
    return 0;
}

void CrashHandler::InitExceptionHandler()
{
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
}
