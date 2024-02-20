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
#include "JsonReader.h"
#include "ModelManager.h"
#include "PreviewerEngineLog.h"
#include "VirtualScreen.h"
#include "CommandParser.h"

using namespace std;

const string CommandLineInterface::COMMAND_VERSION = "1.0.1";
bool CommandLineInterface::isFirstWsSend = true;
bool CommandLineInterface::isPipeConnected = false;
CommandLineInterface::CommandLineInterface() : socket(nullptr) {}

CommandLineInterface::~CommandLineInterface() {}

void CommandLineInterface::InitPipe(const string name)
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

void CommandLineInterface::SendJsonData(const Json::Value& value)
{
    *(GetInstance().socket) << value.toStyledString();
}

void CommandLineInterface::SendJSHeapMemory(size_t total, size_t alloc, size_t peak) const
{
    Json::Value result;
    result["version"] = COMMAND_VERSION;
    result["property"] = "memoryUsage";
    Json::Value memory;
    memory["totalBytes"] = static_cast<Json::UInt64>(total);
    memory["allocBytes"] = static_cast<Json::UInt64>(alloc);
    memory["peakAllocBytes"] = static_cast<Json::UInt64>(peak);
    result["result"] = memory;
    if (socket == nullptr) {
        ELOG("CommandLineInterface::SendJSHeapMemory socket is null");
        return;
    }
    *socket << result.toStyledString();
}

void CommandLineInterface::SendWebsocketStartupSignal() const
{
    Json::Value result;
    Json::Value args;
    result["MessageType"] = "imageWebsocket";
    args["port"] = VirtualScreen::webSocketPort;
    result["args"] = args;
    *socket << result.toStyledString();
}

void CommandLineInterface::ProcessCommand() const
{
    string message; /* NOLINT */
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
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    if (reader == nullptr) {
        FLOG("CommandLineInterface: CharReader memory allocation failed.");
    }

    Json::Value jsonData;
    std::string errors; /* NOLINT */

    bool parsingSuccessful = reader->parse(message.c_str(), message.c_str() + message.size(), &jsonData, &errors);
    delete reader;
    reader = nullptr;

    if (!ProcessCommandValidate(parsingSuccessful, jsonData, errors)) {
        return;
    }

    CommandLine::CommandType type = GetCommandType(jsonData["type"].asString());
    if (type == CommandLine::CommandType::INVALID) {
        return;
    }

    string command = jsonData["command"].asString();
    if (CommandParser::GetInstance().IsStaticCard() && IsStaticIgnoreCmd(command)) {
        return;
    }
    std::unique_ptr<CommandLine> commandLine =
        CommandLineFactory::CreateCommandLine(command, type, jsonData["args"], *socket);
    if (commandLine == nullptr) {
        ELOG("Unsupported command");
        return;
    }
    commandLine->CheckAndRun();
}

bool CommandLineInterface::ProcessCommandValidate(bool parsingSuccessful,
                                                  const Json::Value& jsonData,
                                                  const std::string& errors) const
{
    if (!parsingSuccessful) {
        ELOG("Failed to parse the JSON, errors: %s", errors.c_str());
        return false;
    }

    if (!jsonData.isObject()) {
        ELOG("Command is not a object!");
        return false;
    }

    if (!jsonData.isMember("type") || !jsonData.isMember("command") || !jsonData.isMember("version")) {
        ELOG("Command error!");
        return false;
    }

    if (!regex_match(jsonData["version"].asString(), regex("(([0-9]|([1-9]([0-9]*))).){2}([0-9]|([1-9]([0-9]*)))"))) {
        ELOG("Invalid command version!");
        return false;
    }
    return true;
}

CommandLine::CommandType CommandLineInterface::GetCommandType(string name) const
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

void CommandLineInterface::ApplyConfig(const Json::Value& val) const
{
    const string set("setting");
    if (val.isMember(set)) {
        Json::Value versionMembers = val[set];
        if (!versionMembers.isObject()) {
            return;
        }

        Json::Value::Members versions = versionMembers.getMemberNames();

        for (Json::Value::Members::iterator viter = versions.begin(); viter != versions.end(); viter++) {
            string version = *viter;
            Json::Value commands = versionMembers[version];
            if (!commands.isObject()) {
                continue;
            }
            Json::Value::Members members = commands.getMemberNames();

            ApplyConfigMembers(commands, members);
        }
    }
}

void CommandLineInterface::ApplyConfigMembers(const Json::Value& commands,
                                              const Json::Value::Members& members) const
{
    for (Json::Value::Members::const_iterator iter = members.begin(); iter != members.end(); iter++)  {
        string key = *iter;
        if (!commands[key].isObject() || !commands[key].isMember("args") || !commands[key]["args"].isObject()) {
            ELOG("Invalid JSON: %s", commands[key].asString().c_str());
            continue;
        }
        std::unique_ptr<CommandLine> command =
            CommandLineFactory::CreateCommandLine(key, CommandLine::CommandType::SET, commands[key]["args"], *socket);
        ApplyConfigCommands(key, command);
    }
}

void CommandLineInterface::ApplyConfigCommands(const string& key,
                                               const unique_ptr<CommandLine>& command) const
{
    if (command == nullptr) {
        ELOG("Unsupported configuration: %s", key.c_str());
        return;
    }

    if (command->IsArgValid()) {
        command->RunSet();
    }
}

void CommandLineInterface::Init(string pipeBaseName)
{
    CommandLineFactory::InitCommandMap();
    InitPipe(pipeBaseName);
}

void CommandLineInterface::ReadAndApplyConfig(string path) const
{
    if (path.empty()) {
        return;
    }
    string jsonStr = JsonReader::ReadFile(path);
    Json::Value val = JsonReader::ParseJsonData(jsonStr);
    ApplyConfig(val);
}

void CommandLineInterface::CreatCommandToSendData(const string commandName,
                                                  const Json::Value jsonData,
                                                  const string type) const
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

bool CommandLineInterface::IsStaticIgnoreCmd(const string cmd) const
{
    auto it = std::find(staticIgnoreCmd.begin(), staticIgnoreCmd.end(), cmd);
    if (it != staticIgnoreCmd.end()) {
        return false;
    } else {
        return true;
    }
}
