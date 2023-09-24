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

#include "SystemCapability.h"

#include <fstream>

#include "CommandParser.h"
#include "FileSystem.h"
#include "PreviewerEngineLog.h"
#include "json/json.h"

using namespace std;
SystemCapability& SystemCapability::GetInstance()
{
    static SystemCapability instance;
    return instance;
}

bool SystemCapability::HasSystemCapability(const char* sysCapName)
{
    string capName = sysCapName;
    if (capabilities.find(sysCapName) == capabilities.end()) {
        return false;
    }
    return capabilities[sysCapName];
}

SystemCapability::SystemCapability()
{
    ReadCapability();
}

void SystemCapability::ReadCapability()
{
    std::string separator = FileSystem::GetSeparator();
    std::string filePath = FileSystem::GetApplicationPath() + separator + ".." + separator + "config" + separator +
                           "system_capability.json";
    ifstream inFile(filePath);
    if (!inFile.is_open()) {
        ELOG("Open capability file failed.");
    }
    string jsonStr((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    Json::Value val;
    Json::CharReaderBuilder builder;
    Json::CharReader* charReader = builder.newCharReader();
    if (charReader == nullptr) {
        ELOG("main: CharReader memory allocation failed.");
    }
    std::unique_ptr<Json::CharReader> reader(charReader);
    if (reader.get() == nullptr) {
        ELOG("main: CharReader memory allocation failed.");
    }
    string message;
    if (!reader->parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), &val, &message)) {
        ELOG("Failed to parse the capability, errors: %s", message.c_str());
    }
    if (val["systemCapability"].empty() || !val["systemCapability"].isArray()) {
        ELOG("Empty systemCapability json object: %s", val["systemCapability"].toStyledString().c_str());
        return;
    }
    val = val["systemCapability"];
    for (Json::ValueIterator iter = val.begin(); iter != val.end(); iter++) {
        Json::Value cap = *iter;
        if (!cap.isMember("name") || !cap.isMember("register-on-startup")) {
            ELOG("Invalid systemCapability json object");
        }
        if (!cap["register-on-startup"].isBool()) {
            ELOG("Invalid systemCapability json object");
        }
        capabilities[cap["name"].asString()] = cap["register"].asBool();
    }
}
