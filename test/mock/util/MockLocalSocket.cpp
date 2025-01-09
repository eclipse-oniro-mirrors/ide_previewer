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

#include "LocalSocket.h"
#include "MockGlobalResult.h"

LocalSocket::LocalSocket() {}

LocalSocket::~LocalSocket() {}

bool LocalSocket::ConnectToServer(std::string name, OpenMode openMode, TransMode transMode)
{
    return true;
}

void LocalSocket::DisconnectFromServer()
{
    g_disconnectFromServer = false;
}

std::string LocalSocket::GetCommandPipeName(std::string baseName) const
{
    return "/tmp/phone_commandPipe";
}

std::string LocalSocket::GetTracePipeName(std::string baseName) const
{
    return "/tmp/phone";
}

const LocalSocket& LocalSocket::operator>>(std::string& data) const
{
    g_input = true;
    return *this;
}

const LocalSocket& LocalSocket::operator<<(const std::string data) const
{
    g_output = true;
    return *this;
}

int64_t LocalSocket::ReadData(char* data, size_t length) const
{
    return length;
}

size_t LocalSocket::WriteData(const void* data, size_t length) const
{
    return length;
}

bool LocalSocket::RunServer(std::string name)
{
    return true;
}
