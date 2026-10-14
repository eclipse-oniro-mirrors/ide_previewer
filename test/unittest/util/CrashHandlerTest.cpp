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
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <memory>
#include <csignal>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>
#include "gtest/gtest.h"
#include "securec.h"
#define private public
#include "CrashHandler.h"

namespace {
    class CrashHandlerTest : public ::testing::Test {
    protected:
        static void SetUpTestCase()
        {
            // 在测试开始前重置信号处理函数为空
            if (signal(SIGSEGV, SIG_DFL) == SIG_ERR) {
                printf("set signal failed");
            }
        }

        static void TearDownTestCase()
        {
            // 在测试结束后重置信号处理函数为空
            if (signal(SIGSEGV, SIG_DFL) == SIG_ERR) {
                printf("set signal failed");
            }
        }

        // 将 stderr 重定向到文件，返回备份的文件描述符，失败返回 -1
        static int RedirectStderrToFile(const std::string& fileName)
        {
            int stderrBackup = dup(STDERR_FILENO);
            if (stderrBackup < 0) {
                return -1;
            }
            int fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                close(stderrBackup);
                return -1;
            }
            if (dup2(fd, STDERR_FILENO) < 0) {
                close(fd);
                close(stderrBackup);
                return -1;
            }
            close(fd);
            return stderrBackup;
        }

        // 用备份的文件描述符恢复 stderr
        static void RestoreStderr(int stderrBackup)
        {
            if (stderrBackup >= 0) {
                dup2(stderrBackup, STDERR_FILENO);
                close(stderrBackup);
            }
        }

        static std::string ReadFileToString(const std::string& fileName)
        {
            std::ifstream file(fileName);
            if (!file.is_open()) {
                return "";
            }
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        }
    };

    TEST_F(CrashHandlerTest, InitExceptionHandlerTest)
    {
        // 保存原始的信号处理函数
        void (*originalHandler)(int) = signal(SIGSEGV, SIG_DFL);
        // init exception handler
        auto richCrashHandler = std::make_unique<CrashHandler>();
        richCrashHandler->InitExceptionHandler();
        // 检查信号处理函数是否被正确注册
        EXPECT_NE(signal(SIGSEGV, SIG_DFL), originalHandler);
    }

    TEST_F(CrashHandlerTest, InitExceptionHandlerTwiceTest)
    {
        CrashHandler crashHandler;
        crashHandler.InitExceptionHandler();
        crashHandler.InitExceptionHandler();
        struct sigaction currentAction;
        ASSERT_EQ(memset_s(&currentAction, sizeof(currentAction), 0, sizeof(currentAction)), EOK);
        ASSERT_EQ(sigaction(SIGSEGV, nullptr, &currentAction), 0);
        EXPECT_EQ(currentAction.sa_handler, &CrashHandler::ApplicationCrashHandler);
        signal(SIGSEGV, SIG_DFL);
    }

    TEST_F(CrashHandlerTest, InitExceptionHandlerRaiseTest)
    {
        CrashHandler crashHandler;
        crashHandler.InitExceptionHandler();
        struct sigaction currentAction;
        ASSERT_EQ(memset_s(&currentAction, sizeof(currentAction), 0, sizeof(currentAction)), EOK);
        ASSERT_EQ(sigaction(SIGSEGV, nullptr, &currentAction), 0);
        // 注册失败时不触发 raise，避免进程被默认行为终止
        ASSERT_EQ(currentAction.sa_handler, &CrashHandler::ApplicationCrashHandler);

        const std::string fileName = "crash_handler_raise_ut";
        int stderrBackup = RedirectStderrToFile(fileName);
        ASSERT_GE(stderrBackup, 0);
        // raise 触发的 SIGSEGV 在处理函数返回后继续执行，不会终止进程
        raise(SIGSEGV);
        RestoreStderr(stderrBackup);
        signal(SIGSEGV, SIG_DFL);

        std::string content = ReadFileToString(fileName);
        std::remove(fileName.c_str());
        EXPECT_NE(content.find("[JsEngine Crash]Engine Crash Info Begin."), std::string::npos);
        EXPECT_NE(content.find("[JsEngine Crash]Error: signal : 0xb"), std::string::npos);
        EXPECT_NE(content.find("[JsEngine Crash]Engine Crash Info End."), std::string::npos);
    }

    TEST_F(CrashHandlerTest, ApplicationCrashHandlerDirectTest)
    {
        const std::string fileName = "crash_handler_stderr_ut";
        int stderrBackup = RedirectStderrToFile(fileName);
        ASSERT_GE(stderrBackup, 0);
        CrashHandler::ApplicationCrashHandler(SIGSEGV);
        RestoreStderr(stderrBackup);

        std::string content = ReadFileToString(fileName);
        std::remove(fileName.c_str());
        EXPECT_NE(content.find("[JsEngine Crash]Engine Crash Info Begin."), std::string::npos);
        EXPECT_NE(content.find("[JsEngine Crash]Error: signal : 0xb"), std::string::npos);
        EXPECT_NE(content.find("[JsEngine Crash]Engine Crash Info End."), std::string::npos);
    }

    TEST_F(CrashHandlerTest, ApplicationCrashHandlerSignalsTest)
    {
        const std::vector<std::pair<int, std::string>> signalCases = {
            {SIGABRT, "0x6"},
            {SIGFPE, "0x8"},
            {SIGILL, "0x4"},
            {0, "0x0"},
        };
        const std::string fileName = "crash_handler_signals_ut";
        for (const auto& caseItem : signalCases) {
            int stderrBackup = RedirectStderrToFile(fileName);
            ASSERT_GE(stderrBackup, 0);
            CrashHandler::ApplicationCrashHandler(caseItem.first);
            RestoreStderr(stderrBackup);
            std::string content = ReadFileToString(fileName);
            EXPECT_NE(content.find("[JsEngine Crash]Engine Crash Info Begin."), std::string::npos);
            EXPECT_NE(content.find("[JsEngine Crash]Error: signal : " + caseItem.second), std::string::npos);
            EXPECT_NE(content.find("[JsEngine Crash]Engine Crash Info End."), std::string::npos);
        }
        std::remove(fileName.c_str());
    }
}