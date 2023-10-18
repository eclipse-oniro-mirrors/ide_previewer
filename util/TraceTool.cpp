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
#include "json.h"
#include "CommandParser.h"
#include "PreviewerEngineLog.h"
#include "TimeTool.h"
#include "LocalSocket.h"

using namespace std;

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
    string name = GetTracePipeName();
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

void TraceTool::SendTraceData(const Json::Value& value)
{
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    ostringstream osStream;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(value, &osStream);
    *(GetInstance().socket) << osStream.str();
}

void TraceTool::HandleTrace(const string msg) const
{
    if (!isReady) {
        ILOG("Trace pipe is not prepared");
        return;
    }
    Json::Value val = GetBaseInfo();
    val["action"] = msg;
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

Json::Value TraceTool::GetBaseInfo() const
{
    Json::Value val;
    val["sid"] = "10007";
    val["detail"]["ProjectId"] = CommandParser::GetInstance().GetProjectID();
    val["detail"]["device"] = CommandParser::GetInstance().GetDeviceType();
    val["detail"]["time"] = TimeTool::GetTraceFormatTime();
    return val;
}

string TraceTool::GetTracePipeName() const
{
    return CommandParser::GetInstance().Value("ts");
}
