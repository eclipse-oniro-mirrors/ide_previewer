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
#include "JsonReader.h"

struct ResolutionParam {
    ResolutionParam(int32_t orignalWidth, int32_t orignalHeight,
        int32_t compressionWidth, int32_t compressionHeight)
    {
        this->orignalWidth = orignalWidth;
        this->orignalHeight = orignalHeight;
        this->compressionWidth = compressionWidth;
        this->compressionHeight = compressionHeight;
    }
    int32_t orignalWidth;
    int32_t orignalHeight;
    int32_t compressionWidth;
    int32_t compressionHeight;
};

struct AvoidRect {
    int32_t posX;
    int32_t posY;
    uint32_t width;
    uint32_t height;

    AvoidRect() : posX(0), posY(0), width(0), height(0) {}

    AvoidRect(int32_t x, int32_t y, uint32_t w, uint32_t h) : posX(x), posY(y), width(w), height(h) {}

    AvoidRect(const AvoidRect& other) : posX(other.posX), posY(other.posY),
        width(other.width), height(other.height) {}

    bool operator==(const AvoidRect& a) const
    {
        return (posX == a.posX && posY == a.posY && width == a.width && height == a.height);
    }

    AvoidRect& operator=(const AvoidRect& other)
    {
        if (this != &other) {
            this->posX = other.posX;
            this->posY = other.posY;
            this->width = other.width;
            this->height = other.height;
        }
        return *this;
    }
};

struct AvoidAreas {
    AvoidRect topRect { 0, 0, 0, 0 };
    AvoidRect leftRect { 0, 0, 0, 0 };
    AvoidRect rightRect { 0, 0, 0, 0 };
    AvoidRect bottomRect { 0, 0, 0, 0 };

    AvoidAreas() : topRect(0, 0, 0, 0), leftRect(0, 0, 0, 0), rightRect(0, 0, 0, 0), bottomRect(0, 0, 0, 0) {}

    AvoidAreas(AvoidRect top, AvoidRect left, AvoidRect right, AvoidRect bottom) : topRect(top),
        leftRect(left), rightRect(right), bottomRect(bottom) {}

    AvoidAreas(const AvoidAreas& other) : topRect(other.topRect), leftRect(other.leftRect),
                                          rightRect(other.rightRect), bottomRect(other.bottomRect) {}

    bool operator==(const AvoidAreas& a) const
    {
        return (topRect == a.topRect && leftRect == a.leftRect &&
            rightRect == a.rightRect && bottomRect == a.bottomRect);
    }

    AvoidAreas& operator=(const AvoidAreas& other)
    {
        if (this != &other) {
            this->topRect = other.topRect;
            this->leftRect = other.leftRect;
            this->rightRect = other.rightRect;
            this->bottomRect = other.bottomRect;
        }
        return *this;
    }
};

class JsApp {
public:
    JsApp& operator=(const JsApp&) = delete;
    JsApp(const JsApp&) = delete;
    virtual void Start() = 0;
    virtual void Restart() = 0;
    virtual void Interrupt() = 0;
    virtual void Stop();
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
    virtual void ResolutionChanged(ResolutionParam&, int32_t, std::string);
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
    virtual void LoadDocument(const std::string, const std::string, const Json2::Value&);
    virtual void FoldStatusChanged(const std::string commandFoldStatus, int32_t width, int32_t height);
    virtual void SetAvoidArea(const AvoidAreas& areas);
    virtual const AvoidAreas GetCurrentAvoidArea() const;
    virtual void InitJsApp();
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
