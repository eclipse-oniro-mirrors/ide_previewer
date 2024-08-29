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

#include "KeyInputImpl.h"
#include <chrono>
#include <mutex>
#include "PreviewerEngineLog.h"
#include "ClipboardHelper.h"
#include "KeyboardHelper.h"
#include "JsAppImpl.h"

using namespace OHOS::MMI;

KeyInputImpl::KeyInputImpl() : KeyInput(), pressedCodes(0)
{
}

KeyInputImpl& KeyInputImpl::GetInstance()
{
    static KeyInputImpl instance;
    return instance;
}

void KeyInputImpl::DispatchOsInputMethodEvent() const
{
    JsAppImpl::GetInstance().DispatchInputMethodEvent(codePoint);
}

void KeyInputImpl::DispatchOsKeyEvent() const
{
    auto keyEvent = std::make_shared<OHOS::MMI::KeyEvent>();
    keyEvent->code = KeyCode(keyCode);
    keyEvent->action = KeyAction(keyAction);
    keyEvent->pressedCodes = pressedCodes;
    keyEvent->timeStamp = std::chrono::high_resolution_clock::now();
    keyEvent->key = keyString.c_str();
    keyEvent->enableCapsLock_ = KeyboardHelper::GetKeyStateByKeyName("CapsLock");
    keyEvent->enableNumLock_ = KeyboardHelper::GetKeyStateByKeyName("NumLock");
    JsAppImpl::GetInstance().DispatchKeyEvent(keyEvent);
}

void KeyInputImpl::SetKeyEvent(const int32_t keyCodeVal, const int32_t keyActionVal,
    const std::vector<int32_t> pressedCodesVal, const std::string keyStrVal)
{
    keyCode = keyCodeVal;
    keyAction = keyActionVal;
    pressedCodes.clear();
    for (int32_t num : pressedCodesVal) {
        pressedCodes.push_back(KeyCode(num));
    }
    keyString = keyStrVal;
}

void KeyInputImpl::SetCodePoint(const unsigned int codePointVal)
{
    codePoint = codePointVal;
}
