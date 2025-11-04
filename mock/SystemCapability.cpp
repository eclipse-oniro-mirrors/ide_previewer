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
#include "JsonReader.h"

SystemCapability& SystemCapability::GetInstance()
{
    static SystemCapability instance;
    return instance;
}

bool SystemCapability::HasSystemCapability(const char* sysCapName)
{
    std::string capName = sysCapName;
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
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        ELOG("Open capability file failed.");
    }
    std::string jsonStr((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    Json2::Value val = JsonReader::ParseJsonData2(jsonStr);
    if (val.IsNull()) {
        std::string message = JsonReader::GetErrorPtr();
        ELOG("Failed to parse the capability, errors: %s", message.c_str());
    }
    if (val["systemCapability"].IsNull() || !val["systemCapability"].IsArray()) {
        ELOG("Empty systemCapability json object: %s", val["systemCapability"].ToStyledString().c_str());
        return;
    }
    Json2::Value val2 = val["systemCapability"];
    for (int i = 0; i < val2.GetArraySize(); i++) {
        Json2::Value cap = val2.GetArrayItem(i);
        if (!cap.IsMember("name") || !cap.IsMember("register-on-startup")) {
            ELOG("Invalid systemCapability json object");
        }
        if (!cap["register-on-startup"].IsBool()) {
            ELOG("Invalid systemCapability json object");
        }
        if (cap.IsMember("register") && cap["register"].IsBool() && cap["name"].IsString()) {
            capabilities[cap["name"].AsString()] = cap["register"].AsBool();
        }
    }
}
