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

#ifndef COMMANDLINEFACTORY_H
#define COMMANDLINEFACTORY_H

#include <memory>

#include "CommandLine.h"

class CommandLineFactory {
public:
    CommandLineFactory();
    ~CommandLineFactory() {}
    static void InitCommandMap();
    static std::unique_ptr<CommandLine> CreateCommandLine(std::string command,
                                                          CommandLine::CommandType type,
                                                          Json::Value args,
                                                          const LocalSocket& socket);

private:
    template <typename T>
    static std::unique_ptr<CommandLine>
        CreateObject(CommandLine::CommandType, const Json::Value&, const LocalSocket& socket);
    using CommandTypeMap = std::map<
        std::string,
        std::unique_ptr<CommandLine> (*)(CommandLine::CommandType, const Json::Value&, const LocalSocket& socket)>;
    static CommandTypeMap typeMap;
};

#endif // COMMANDLINEFACTORY_H
