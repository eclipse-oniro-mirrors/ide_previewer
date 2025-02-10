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

#ifndef LOCALSOCKET_H
#define LOCALSOCKET_H

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#endif // _WIN32

#include "EndianUtil.h"

class LocalSocket {
public:
    enum OpenMode { READ_ONLY = 0, WRITE_ONLY, READ_WRITE };

    enum TransMode { TRANS_BYTE = 0, TRANS_MESSAGE };

    LocalSocket();
    virtual ~LocalSocket();
    LocalSocket& operator=(const LocalSocket&) = delete;
    LocalSocket(const LocalSocket&) = delete;
    bool ConnectToServer(std::string name, OpenMode openMode, TransMode transMode = TRANS_BYTE);
    bool RunServer(std::string name);
    bool ConnectClient(bool wait = true);
    std::string GetCommandPipeName(std::string baseName) const;
    std::string GetImagePipeName(std::string baseName) const;
    std::string GetTracePipeName(std::string baseName) const;
    void DisconnectFromServer();
    int64_t ReadData(char* data, size_t length) const;
    size_t WriteData(const void* data, size_t length) const;

    template <class T, class = typename std::enable_if<std::is_integral<T>::value>::type>
    const LocalSocket& operator<<(const T data) const
    {
        T dataToSend = EndianUtil::ToNetworkEndian<T>(data);
        char* startPos = reinterpret_cast<char*>(&dataToSend);
        char buffer[sizeof(T)];
        std::copy(startPos, startPos + sizeof(dataToSend), buffer);
        WriteData(buffer, sizeof(dataToSend));
        return *this;
    }

    const LocalSocket& operator<<(const std::string data) const;

    const LocalSocket& operator>>(std::string& data) const;

private:
    bool isServer = false;
    bool isConnected = false;
    std::string serverName;
#ifdef _WIN32
    HANDLE pipeHandle;
    DWORD GetWinOpenMode(OpenMode mode) const;
    DWORD GetWinTransMode(TransMode mode) const;
#else
    int socketHandle;
#endif // _WIN32
};

#endif // LOCALSOCKET_H
