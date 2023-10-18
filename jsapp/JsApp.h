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

#ifndef JSAPP_H
#define JSAPP_H

#include <atomic>
#include <string>
#include <thread>
#include <vector>
#include "json.h"

class JsApp {
public:
    JsApp& operator=(const JsApp&) = delete;
    JsApp(const JsApp&) = delete;
    virtual void Start() = 0;
    virtual void Restart() = 0;
    virtual void Interrupt() = 0;
    void Stop();
    void SetJsAppPath(const std::string& value);
    void SetUrlPath(const std::string& value);
    void SetPipeName(const std::string& name);
    void SetPipePort(const std::string& port);
    void SetBundleName(const std::string& name);
    void SetRunning(bool flag);
    bool GetRunning();
    void SetIsDebug(bool value);
    void SetDebugServerPort(uint16_t value);
    void SetJSHeapSize(uint32_t size);
    virtual std::string GetJSONTree();
    virtual std::string GetDefaultJSONTree();
    virtual void OrientationChanged(std::string commandOrientation);
    virtual void ResolutionChanged(int32_t, int32_t, int32_t, int32_t, int32_t);
    virtual void SetArgsColorMode(const std::string& value);
    virtual void SetArgsAceVersion(const std::string& value);
    virtual std::string GetOrientation() const;
    virtual std::string GetColorMode() const;
    virtual void ColorModeChanged(const std::string commandColorMode);
    static bool IsLiteDevice(std::string deviceType);
    virtual void ReloadRuntimePage(const std::string);
    virtual void SetScreenDensity(const std::string value);
    virtual void SetConfigChanges(const std::string value);
    virtual bool MemoryRefresh(const std::string) const;
    virtual void LoadDocument(const std::string, const std::string, const Json::Value);

protected:
    JsApp();
    virtual ~JsApp() {};
    std::string pipeName;
    std::string pipePort;
    std::string jsAppPath;
    std::string bundleName;
    std::string urlPath;
    std::atomic<bool> isFinished;
    std::atomic<bool> isRunning;
    bool isDebug;
    uint16_t debugServerPort;
    uint32_t jsHeapSize;
    std::string colorMode;
    std::string orientation;
    std::string aceVersion;
    std::string screenDensity;
    std::string configChanges;
};

#endif // JSAPP_H
