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


#include "CommandLineInterface.h"

#include <chrono>
#include <regex>

#include "CommandLine.h"
#include "CommandLineFactory.h"
#include "ModelManager.h"
#include "PreviewerEngineLog.h"
#include "VirtualScreen.h"
#include "CommandParser.h"

const std::string CommandLineInterface::COMMAND_VERSION = "1.0.1";
bool CommandLineInterface::isFirstWsSend = true;
bool CommandLineInterface::isPipeConnected = false;
CommandLineInterface::CommandLineInterface() : socket(nullptr) {}

CommandLineInterface::~CommandLineInterface() {}

void CommandLineInterface::InitPipe(const std::string name)
{
    if (socket != nullptr) {
        socket.reset();
        ELOG("CommandLineInterface::InitPipe socket is not null");
    }

    socket = std::make_unique<LocalSocket>();
    if (socket == nullptr) {
        FLOG("CommandLineInterface::Connect socket memory allocation failed!");
    }

    if (!socket->ConnectToServer(socket->GetCommandPipeName(name), LocalSocket::READ_WRITE)) {
        FLOG("CommandLineInterface command pipe connect failed");
    }
    isPipeConnected  = true;
}

CommandLineInterface& CommandLineInterface::GetInstance()
{
    static CommandLineInterface instance; /* NOLINT */
    return instance;
}

void CommandLineInterface::SendJsonData(const Json2::Value& value)
{
    *(GetInstance().socket) << value.ToStyledString();
}

void CommandLineInterface::SendJSHeapMemory(size_t total, size_t alloc, size_t peak) const
{
    Json2::Value result = JsonReader::CreateObject();
    result.Add("version", COMMAND_VERSION.c_str());
    result.Add("property", "memoryUsage");
    Json2::Value memory = JsonReader::CreateObject();
    memory.Add("totalBytes", static_cast<double>(total));
    memory.Add("allocBytes", static_cast<double>(alloc));
    memory.Add("peakAllocBytes", static_cast<double>(peak));
    result.Add("result", memory);
    if (socket == nullptr) {
        ELOG("CommandLineInterface::SendJSHeapMemory socket is null");
        return;
    }
    *socket << result.ToStyledString();
}

void CommandLineInterface::SendWebsocketStartupSignal() const
{
    Json2::Value result = JsonReader::CreateObject();
    Json2::Value args = JsonReader::CreateObject();
    result.Add("MessageType", "imageWebsocket");
    args.Add("port", VirtualScreen::webSocketPort.c_str());
    result.Add("args", args);
    *socket << result.ToStyledString();
}

void CommandLineInterface::ProcessCommand() const
{
    std::string message; /* NOLINT */
    if (socket == nullptr) {
        ELOG("CommandLineInterface::ProcessCommand socket is null");
        return;
    }
    if (isPipeConnected && VirtualScreen::isWebSocketListening && isFirstWsSend) {
        isFirstWsSend = false;
        SendWebsocketStartupSignal();
    }
    *socket >> message;
    if (message.empty()) {
        return;
    }

    ProcessCommandMessage(message);
}

void CommandLineInterface::ProcessCommandMessage(std::string message) const
{
    ILOG("***cmd*** message:%s", message.c_str());
    Json2::Value jsonData = JsonReader::ParseJsonData2(message);
    std::string errors; /* NOLINT */
    bool parsingSuccessful = jsonData.IsNull() ? false : true;
    if (!parsingSuccessful) {
        errors = JsonReader::GetErrorPtr();
    }

    if (!ProcessCommandValidate(parsingSuccessful, jsonData, errors)) {
        return;
    }

    CommandLine::CommandType type = GetCommandType(jsonData["type"].AsString());
    if (type == CommandLine::CommandType::INVALID) {
        return;
    }

    std::string command = jsonData["command"].AsString();
    if (CommandParser::GetInstance().IsStaticCard() && IsStaticIgnoreCmd(command)) {
        return;
    }
    Json2::Value val = jsonData["args"];
    std::unique_ptr<CommandLine> commandLine =
        CommandLineFactory::CreateCommandLine(command, type, val, *socket);
    if (commandLine == nullptr) {
        ELOG("Unsupported command");
        return;
    }
    commandLine->CheckAndRun();
}

bool CommandLineInterface::ProcessCommandValidate(bool parsingSuccessful,
                                                  const Json2::Value& jsonData,
                                                  const std::string& errors) const
{
    if (!parsingSuccessful) {
        ELOG("Failed to parse the JSON, errors: %s", errors.c_str());
        return false;
    }

    if (!jsonData.IsObject()) {
        ELOG("Command is not a object!");
        return false;
    }

    if (!jsonData.IsMember("type") || !jsonData.IsMember("command") || !jsonData.IsMember("version")) {
        ELOG("Command error!");
        return false;
    }

    if (!jsonData["version"].IsString() || !regex_match(jsonData["version"].AsString(),
        std::regex("(([0-9]|([1-9]([0-9]*))).){2}([0-9]|([1-9]([0-9]*)))"))) {
        ELOG("Invalid command version!");
        return false;
    }
    return true;
}

CommandLine::CommandType CommandLineInterface::GetCommandType(std::string name) const
{
    CommandLine::CommandType type = CommandLine::CommandType::INVALID;
    if (name == "set") {
        type = CommandLine::CommandType::SET;
    } else if (name == "get") {
        type = CommandLine::CommandType::GET;
    } else if (name == "action") {
        type = CommandLine::CommandType::ACTION;
    } else {
        ELOG("Command type invalid!");
    }
    return type;
}

void CommandLineInterface::ApplyConfig(const Json2::Value& val) const
{
    const std::string set("setting");
    if (val.IsMember(set.c_str())) {
        Json2::Value versionMembers = val[set];
        if (!versionMembers.IsObject()) {
            return;
        }

        Json2::Value::Members versions = versionMembers.GetMemberNames();

        for (Json2::Value::Members::iterator viter = versions.begin(); viter != versions.end(); viter++) {
            std::string version = *viter;
            Json2::Value commands = versionMembers[version];
            if (!commands.IsObject()) {
                continue;
            }
            Json2::Value::Members members = commands.GetMemberNames();

            ApplyConfigMembers(commands, members);
        }
    }
}

void CommandLineInterface::ApplyConfigMembers(const Json2::Value& commands,
                                              const Json2::Value::Members& members) const
{
    for (Json2::Value::Members::const_iterator iter = members.begin(); iter != members.end(); iter++)  {
        std::string key = *iter;
        if (!commands[key].IsObject() || !commands[key].IsMember("args") || !commands[key]["args"].IsObject()) {
            ELOG("Invalid JSON: %s", commands[key].AsString().c_str());
            continue;
        }
        Json2::Value val = commands[key]["args"];
        std::unique_ptr<CommandLine> command =
            CommandLineFactory::CreateCommandLine(key, CommandLine::CommandType::SET, val, *socket);
        ApplyConfigCommands(key, command);
    }
}

void CommandLineInterface::ApplyConfigCommands(const std::string& key,
                                               const std::unique_ptr<CommandLine>& command) const
{
    if (command == nullptr) {
        ELOG("Unsupported configuration: %s", key.c_str());
        return;
    }

    if (command->IsArgValid()) {
        command->RunSet();
    }
}

void CommandLineInterface::Init(std::string pipeBaseName)
{
    CommandLineFactory::InitCommandMap();
    InitPipe(pipeBaseName);
}

void CommandLineInterface::ReadAndApplyConfig(std::string path) const
{
    if (path.empty()) {
        return;
    }
    std::string jsonStr = JsonReader::ReadFile(path);
    Json2::Value val = JsonReader::ParseJsonData2(jsonStr);
    ApplyConfig(val);
}

void CommandLineInterface::CreatCommandToSendData(const std::string commandName,
                                                  const Json2::Value& jsonData,
                                                  const std::string type) const
{
    CommandLine::CommandType commandType = GetCommandType(type);
    std::unique_ptr<CommandLine> commandLine =
        CommandLineFactory::CreateCommandLine(commandName, commandType, jsonData, *socket);
    if (commandLine == nullptr) {
        ELOG("Unsupported CreatCommandToSendData: %s", commandName.c_str());
        return;
    }
    commandLine->RunAndSendResultToManager();
}

bool CommandLineInterface::IsStaticIgnoreCmd(const std::string cmd) const
{
    auto it = std::find(staticIgnoreCmd.begin(), staticIgnoreCmd.end(), cmd);
    if (it != staticIgnoreCmd.end()) {
        return false;
    } else {
        return true;
    }
}
