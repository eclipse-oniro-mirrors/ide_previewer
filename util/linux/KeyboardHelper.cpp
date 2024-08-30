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

#include "KeyboardHelper.h"
#include <X11/XKBlib.h>

short KeyboardHelper::GetKeyStateByKeyName(const std::string& keyName)
{
    std::string name;
    if (keyName == "CapsLock") {
        name = "Caps Lock";
    } else if (keyName == "NumLock") {
        name = "Num Lock";
    } else {
        return -1;
    }
    Bool state;
    Display* display = XOpenDisplay((char*)0);
    Atom capsLock = XInternAtom(display, name.c_str(), False);
    XkbGetNamedIndicator(display, capsLock, NULL, &state, NULL, NULL);
    return state ? 1 : 0;
}