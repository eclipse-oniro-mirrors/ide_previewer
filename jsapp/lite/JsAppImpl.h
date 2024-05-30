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

#ifndef JSAPPIMPL_H
#define JSAPPIMPL_H

#include <atomic>
#include <thread>

#include "CppTimer.h"
#include "JsApp.h"
#include "js_ability.h"

class JsAppImpl : public JsApp {
public:
    JsAppImpl();
    virtual ~JsAppImpl();
    JsAppImpl& operator=(const JsAppImpl&) = delete;
    JsAppImpl(const JsAppImpl&) = delete;

    static JsAppImpl& GetInstance();
    void Start() override;
    void Restart() override;
    void Interrupt() override;
    void InitJsApp() override;
    static const uint8_t FONT_SIZE_DEFAULT = 30;

private:
    std::atomic<bool> isInterrupt;
    const long TASK_HANDLE_TIMER_INTERVAL = 15;
    const long DEVICE_CHECK_TIMER_INTERVAL = 100;
    const long JS_CHECK_TIMER_INTERVAL = 1000;
    std::unique_ptr<CppTimer> taskHandleTimer;
    std::unique_ptr<CppTimer> deviceCheckTimer;
    std::unique_ptr<CppTimer> jsCheckTimer;
    std::unique_ptr<OHOS::ACELite::JSAbility> jsAbility;
    std::unique_ptr<std::thread> jsThread;

    void ThreadCallBack();
    void InitTimer();
    void StartJsApp();
};

#endif // JSAPPIMPL_H
