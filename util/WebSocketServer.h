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

#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <thread>
#include <csignal>
#include <mutex>
#include <string>
#include "libwebsockets.h"

class WebSocketServer {
public:
    WebSocketServer& operator=(const WebSocketServer&) = delete;
    WebSocketServer(const WebSocketServer&) = delete;
    static WebSocketServer& GetInstance();
    void SetServerPort(int port);
    void SetSid(const std::string curSid);
    static int ProtocolCallback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);
    void StartWebsocketListening();
    void Run();
    size_t WriteData(unsigned char* data, size_t length);
    enum class WebSocketState { INIT = -1, UNWRITEABLE = 0, WRITEABLE = 1 };
    static WebSocketState webSocketWritable;
    static uint8_t* firstImageBuffer;
    static uint64_t firstImagebufferSize;
    std::mutex mutex;

private:
    WebSocketServer();
    virtual ~WebSocketServer();
    static bool CheckSid(struct lws* wsi);
    static void SignalHandler(int sig);
    std::unique_ptr<std::thread> serverThread;
    int serverPort;
    const char* serverHostname = "127.0.0.1";
    int websocketMaxConn = 1024;
    static lws* webSocket;
    static volatile sig_atomic_t interrupted;
    static int8_t* receivedMessage;
    static const int MAX_PAYLOAD_SIZE = 6400000;
    static const int WEBSOCKET_SERVER_TIMEOUT = 1000;
    struct lws_protocols protocols[2];
    std::string sid;
    static constexpr int sidMaxLength = 256;
};

#endif // WEBSOCKETSERVER_H
