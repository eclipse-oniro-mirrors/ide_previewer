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

#ifndef VIRTUALMESSAGEIMPL_H
#define VIRTUALMESSAGEIMPL_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "ability_kit.h"

#include "VirtualMessage.h"

class VirtualMessageImpl : public VirtualMessage {
public:
    static VirtualMessageImpl& GetInstance();
    void RegistBundle(const char* bundleAppName, SuccessCallback success, FailCallback fail);
    void UnregistBundle(const char* bundleAppName);
    void SendVirtualMessage(MessageInfo info) override;

private:
    VirtualMessageImpl() {}
    ~VirtualMessageImpl() {}

    std::mutex mutex;
    std::map<std::string, std::pair<SuccessCallback, FailCallback>> callBacks;

    std::vector<char> StringToCharVector(std::string str) const;
};

#endif // VIRTUALMESSAGEIMPL_H
