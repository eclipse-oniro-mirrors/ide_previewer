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

#ifndef COMMAND_PARSE_H
#define COMMAND_PARSE_H

#include <string>
#include <vector>
#include <map>

namespace fuzztest {
    class CommandParse {
    public:
        void CreateAndExecuteCommand(std::map<std::string, std::string> dataMap);
    private:
        void Execute(std::string& commond, std::string& jsonArgsStr, int& typeIndex,
            uint64_t& index, bool changeType);
        std::vector<std::string> types = {"get", "set", "action"};
    };
}

#endif  // COMMAND_PARSE_H