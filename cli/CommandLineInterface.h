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

#ifndef COMMANDLINEINTERFACE_H
#define COMMANDLINEINTERFACE_H

#include <memory>
#include <vector>

#include "CommandLine.h"
#include "LocalSocket.h"

class CommandLineInterface {
public:
    CommandLineInterface(const CommandLineInterface&) = delete;
    CommandLineInterface& operator=(const CommandLineInterface&) = delete;
    void InitPipe(const std::string name);
    static CommandLineInterface& GetInstance();
    static void SendJsonData(const Json2::Value&);
    void SendJSHeapMemory(size_t total, size_t alloc, size_t peak) const;
    void SendWebsocketStartupSignal() const;
    void ProcessCommand() const;
    void ProcessCommandMessage(std::string message) const;
    void ApplyConfig(const Json2::Value& val) const;
    void ApplyConfigMembers(const Json2::Value& commands, const Json2::Value::Members& members) const;
    void ApplyConfigCommands(const std::string& key, const std::unique_ptr<CommandLine>& command) const;
    void Init(std::string pipeBaseName);
    void ReadAndApplyConfig(std::string path) const;
    void CreatCommandToSendData(const std::string, const Json2::Value&, const std::string) const;

    const static std::string COMMAND_VERSION;

private:
    explicit CommandLineInterface();
    virtual ~CommandLineInterface();
    bool ProcessCommandValidate(bool parsingSuccessful, const Json2::Value& jsonData, const std::string& errors) const;
    CommandLine::CommandType GetCommandType(std::string) const;
    std::unique_ptr<LocalSocket> socket;
    const static uint32_t MAX_COMMAND_LENGTH = 128;
    static bool isFirstWsSend;
    static bool isPipeConnected;
    std::vector<std::string> staticIgnoreCmd = { "ResolutionSwitch", "exit", "Language", "SupportedLanguages" };
    bool IsStaticIgnoreCmd(const std::string cmd) const;
};

#endif // COMMANDLINEINTERFACE_H
