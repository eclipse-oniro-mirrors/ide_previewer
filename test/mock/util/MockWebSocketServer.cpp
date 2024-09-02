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

#include <atomic>
#include "WebSocketServer.h"
#include "MockGlobalResult.h"
using namespace std;

lws* WebSocketServer::webSocket = nullptr;
std::atomic<bool> WebSocketServer::interrupted = false;
WebSocketServer::WebSocketState WebSocketServer::webSocketWritable = WebSocketState::INIT;
uint8_t* WebSocketServer::firstImageBuffer = nullptr;
uint64_t WebSocketServer::firstImagebufferSize = 0;

WebSocketServer::WebSocketServer() : serverThread(nullptr), serverPort(0) {}

WebSocketServer::~WebSocketServer() {}

WebSocketServer& WebSocketServer::GetInstance()
{
    static WebSocketServer server;
    return server;
}

void WebSocketServer::SetServerPort(int port)
{
    serverPort = port;
}

int WebSocketServer::ProtocolCallback(struct lws* wsi,
                                      enum lws_callback_reasons reason,
                                      void* user,
                                      void* in,
                                      size_t len)
{
    return 0;
}

void WebSocketServer::SignalHandler(int sig)
{
    interrupted = true;
}

void WebSocketServer::StartWebsocketListening() {}

void WebSocketServer::Run()
{
    g_run = true;
}

size_t WebSocketServer::WriteData(unsigned char* data, size_t length)
{
    g_writeData = true;
    return length;
}