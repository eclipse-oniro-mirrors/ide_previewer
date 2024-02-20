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

#ifndef TRACETOOL_H
#define TRACETOOL_H

#include <memory>

namespace Json {
    class Value;
}

class LocalSocket;

class TraceTool {
public:
    static TraceTool& GetInstance();
    static void SendTraceData(const Json::Value& value);
    void InitPipe();
    void HandleTrace(const std::string msg) const;

private:
    TraceTool();
    ~TraceTool();
    std::unique_ptr<LocalSocket> socket;
    Json::Value GetBaseInfo() const;
    std::string GetTracePipeName() const;
    bool isReady;
};

#endif // TRACETOOL_H
