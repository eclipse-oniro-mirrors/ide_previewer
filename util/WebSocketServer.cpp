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

#include <thread>
#include "CommandLineInterface.h"
#include "PreviewerEngineLog.h"
#include "WebSocketServer.h"
using namespace std;

lws* WebSocketServer::webSocket = nullptr;
volatile sig_atomic_t WebSocketServer::interrupted = 0;
WebSocketServer::WebSocketState WebSocketServer::webSocketWritable = WebSocketState::INIT;
uint8_t* WebSocketServer::firstImageBuffer = nullptr;
uint64_t WebSocketServer::firstImagebufferSize = 0;
int8_t* WebSocketServer::receivedMessage = nullptr;

WebSocketServer::WebSocketServer() : serverThread(nullptr), serverPort(0)
{
    protocols[0] = {"ws", WebSocketServer::ProtocolCallback, 0, MAX_PAYLOAD_SIZE};
    protocols[1] = {NULL, NULL, 0, 0};
}

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
    switch (reason) {
        case LWS_CALLBACK_PROTOCOL_INIT:
            ILOG("Engine Websocket protocol init");
            break;
        case LWS_CALLBACK_ESTABLISHED:
            ILOG("Websocket client connect");
            webSocket = wsi;
            lws_callback_on_writable(wsi);
            break;
        case LWS_CALLBACK_RECEIVE:
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
            ILOG("Engine websocket server writeable");
            if (firstImagebufferSize > 0 && webSocketWritable == WebSocketState::UNWRITEABLE) {
                ILOG("Send last image after websocket reconnected");
                std::lock_guard<std::mutex> guard(WebSocketServer::GetInstance().mutex);
                lws_write(wsi,
                          firstImageBuffer + LWS_PRE,
                          firstImagebufferSize,
                          LWS_WRITE_BINARY);
            }
            webSocketWritable = WebSocketState::WRITEABLE;
            break;
        case LWS_CALLBACK_CLOSED:
            ILOG("Websocket client connection closed");
            webSocketWritable = WebSocketState::UNWRITEABLE;
            break;
        default:
            break;
    }
    return 0;
}

void WebSocketServer::SignalHandler(int sig)
{
    interrupted = 1;
}

void WebSocketServer::StartWebsocketListening()
{
    const auto sig = signal(SIGINT, SignalHandler);
    if (sig == SIG_ERR) {
        ELOG("StartWebsocketListening failed");
        return;
    }
    ILOG("Begin to start websocket listening!");
    struct lws_context_creation_info contextInfo = {0};
    contextInfo.port = serverPort;
    contextInfo.iface = serverHostname;
    contextInfo.protocols = protocols;
    contextInfo.ip_limit_wsi = websocketMaxConn;
    contextInfo.options  = LWS_SERVER_OPTION_VALIDATE_UTF8;
    struct lws_context* context = lws_create_context(&contextInfo);
    if (context == nullptr) {
        ELOG("WebSocketServer::StartWebsocketListening context memory allocation failed");
        return;
    }
    while (interrupted == 0) {
        if (lws_service(context, WEBSOCKET_SERVER_TIMEOUT)) {
            interrupted = 1;
        }
    }
    lws_context_destroy(context);
}

void WebSocketServer::Run()
{
    serverThread = std::make_unique<std::thread>([this]() {
        this->StartWebsocketListening();
    });
    if (serverThread == nullptr) {
        ELOG("WebSocketServer::Start serverThread memory allocation failed");
    }
    serverThread->detach();
}

size_t WebSocketServer::WriteData(unsigned char* data, size_t length)
{
    while (webSocketWritable != WebSocketState::WRITEABLE) {
        this_thread::sleep_for(chrono::milliseconds(1));
    }
    if (webSocket != nullptr && webSocketWritable == WebSocketState::WRITEABLE) {
        return lws_write(webSocket, data, length, LWS_WRITE_BINARY);
    }
    return 0;
}
