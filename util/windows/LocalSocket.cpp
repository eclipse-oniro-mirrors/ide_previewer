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

#include "LocalSocket.h"

#include "PreviewerEngineLog.h"

LocalSocket::LocalSocket() : pipeHandle(nullptr) {}

LocalSocket::~LocalSocket() {}

bool LocalSocket::ConnectToServer(std::string name, LocalSocket::OpenMode openMode, TransMode transMode)
{
    std::wstring tempName = std::wstring(name.begin(), name.end());

    DWORD openModeWin = GetWinOpenMode(openMode);
    pipeHandle = CreateFileW(tempName.c_str(), openModeWin, 0, nullptr, OPEN_EXISTING, 0, NULL);
    if (pipeHandle == INVALID_HANDLE_VALUE) {
        ELOG("LocalSocket::ConnectToServer CreateFileW failed: %d", GetLastError());
        return false;
    }

    DWORD tranMode = GetWinTransMode(transMode);
    if (!SetNamedPipeHandleState(pipeHandle, &tranMode, nullptr, nullptr)) {
        ELOG("LocalSocket::ConnectToServer SetNamedPipeHandleState failed: %d", GetLastError());
        return false;
    }

    return true;
}

std::string LocalSocket::GetTracePipeName(std::string baseName) const
{
    return std::string("\\\\.\\pipe\\") + baseName;
}

std::string LocalSocket::GetCommandPipeName(std::string baseName) const
{
    return std::string("\\\\.\\pipe\\") + baseName + "_commandPipe";
}

std::string LocalSocket::GetImagePipeName(std::string baseName) const
{
    return std::string("\\\\.\\pipe\\") + baseName + "_imagePipe";
}

void LocalSocket::DisconnectFromServer()
{
    CloseHandle(pipeHandle);
}

int64_t LocalSocket::ReadData(char* data, size_t length) const
{
    if (length > UINT32_MAX) {
        ELOG("LocalSocket::ReadData length must < %d", UINT32_MAX);
        return -1;
    }

    DWORD readSize = 0;
    if (!PeekNamedPipe(pipeHandle, nullptr, 0, nullptr, &readSize, nullptr)) {
        return 0;
    }

    if (readSize == 0) {
        return 0;
    }

    if (!ReadFile(pipeHandle, data, static_cast<DWORD>(length), &readSize, NULL)) {
        DWORD error = GetLastError();
        ELOG("LocalSocket::ReadData ReadFile failed: %d", error);
        return 0 - static_cast<int64_t>(error);
    }
    return readSize;
}

size_t LocalSocket::WriteData(const void* data, size_t length) const
{
    if (length > UINT32_MAX) {
        ELOG("LocalSocket::WriteData length must < %d", UINT32_MAX);
        return 0;
    }

    DWORD writeSize = 0;
    if (!WriteFile(pipeHandle, data, static_cast<DWORD>(length), &writeSize, nullptr)) {
        DWORD error = GetLastError();
        ELOG("LocalSocket::WriteData WriteFile failed: %d", error);
        return 0 - static_cast<size_t>(error);
    }
    return writeSize;
}

const LocalSocket& LocalSocket::operator<<(const std::string data) const
{
    WriteData(data.c_str(), data.length() + 1);
    return *this;
}

const LocalSocket& LocalSocket::operator>>(std::string& data) const
{
    char c = '\255';
    while (c != '\0' && ReadData(&c, 1) > 0) {
        data.push_back(c);
    }
    return *this;
}

DWORD LocalSocket::GetWinOpenMode(LocalSocket::OpenMode mode) const
{
    DWORD openModeWin = GENERIC_READ;
    switch (mode) {
        case READ_ONLY:
            openModeWin = GENERIC_READ;
            break;
        case WRITE_ONLY:
            openModeWin = GENERIC_WRITE;
            break;
        case READ_WRITE:
            openModeWin = GENERIC_READ | GENERIC_WRITE;
    }
    return openModeWin;
}

DWORD LocalSocket::GetWinTransMode(LocalSocket::TransMode mode) const
{
    DWORD transMode = PIPE_READMODE_BYTE;
    switch (mode) {
        case TRANS_BYTE:
            transMode = PIPE_READMODE_BYTE;
            break;
        case TRANS_MESSAGE:
            transMode = PIPE_READMODE_MESSAGE;
            break;
    }
    return transMode;
}
