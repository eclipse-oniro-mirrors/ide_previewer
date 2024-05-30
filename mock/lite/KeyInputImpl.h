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

#ifndef KEYINPUTIMPL_H
#define KEYINPUTIMPL_H

#include "KeyInput.h"

class KeyInputImpl : public KeyInput {
public:
    static KeyInputImpl& GetInstance();
private:
    KeyInputImpl();
    virtual ~KeyInputImpl() {};
};

#endif // KEYINPUTIMPL_H