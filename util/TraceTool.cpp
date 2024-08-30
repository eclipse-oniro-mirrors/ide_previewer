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

#include "TraceTool.h"
#include "JsonReader.h"
#include "CommandParser.h"
#include "PreviewerEngineLog.h"
#include "TimeTool.h"
#include "LocalSocket.h"

void TraceTool::InitPipe()
{
    if (socket != nullptr) {
        socket.reset();
        ELOG("TraceTool::InitPipe socket is not null");
    }

    socket = std::make_unique<LocalSocket>();
    if (socket == nullptr) {
        FLOG("TraceTool::Connect socket memory allocation failed!");
    }
    std::string name = GetTracePipeName();
    if (!socket->ConnectToServer(socket->GetTracePipeName(name), LocalSocket::READ_WRITE)) {
        ELOG("TraceTool::pipe connect failed");
        return;
    }
    isReady = true;
    ELOG("TraceTool::pipe connect successed");
}

TraceTool& TraceTool::GetInstance()
{
    static TraceTool instance;
    return instance;
}

void TraceTool::SendTraceData(const Json2::Value& value)
{
    *(GetInstance().socket) << value.ToString();
}

void TraceTool::HandleTrace(const std::string msg) const
{
    if (!isReady) {
        ILOG("Trace pipe is not prepared");
        return;
    }
    Json2::Value val = JsonReader::CreateObject();
    val.Add("sid", "10007");
    Json2::Value detail = JsonReader::CreateObject();
    detail.Add("ProjectId", CommandParser::GetInstance().GetProjectID().c_str());
    detail.Add("device", CommandParser::GetInstance().GetDeviceType().c_str());
    detail.Add("time", TimeTool::GetTraceFormatTime().c_str());
    val.Add("detail", detail);
    val.Add("action", msg.c_str());
    SendTraceData(val);
}

TraceTool::TraceTool() : socket(nullptr), isReady(false)
{
    InitPipe();
}

TraceTool::~TraceTool()
{
    if (socket != nullptr) {
        socket->DisconnectFromServer();
        socket = nullptr;
    }
}

std::string TraceTool::GetTracePipeName() const
{
    return CommandParser::GetInstance().Value("ts");
}
