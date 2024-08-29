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

#include "VirtualMessageImpl.h"
#include "PreviewerEngineLog.h"

VirtualMessageImpl& VirtualMessageImpl::GetInstance()
{
    static VirtualMessageImpl instance;
    return instance;
}

void VirtualMessageImpl::RegistBundle(const char* bundleAppName, SuccessCallback success, FailCallback fail)
{
    mutex.lock();
    std::string buildeName(bundleAppName);
    callBacks[buildeName] = std::make_pair(success, fail);
    mutex.unlock();
}

void VirtualMessageImpl::UnregistBundle(const char* bundleAppName)
{
    mutex.lock();
    callBacks.erase(std::string(bundleAppName));
    mutex.unlock();
}

std::vector<char> VirtualMessageImpl::StringToCharVector(std::string str) const
{
    std::vector<char> vec(str.begin(), str.end());
    vec.push_back('\0');
    return vec;
}

void VirtualMessageImpl::SendVirtualMessage(MessageInfo info)
{
    OHOS::FeatureAbilityDataInfo osInfo;
    std::vector<char> deviceId = StringToCharVector(info.deviceID);
    std::vector<char> bundleName = StringToCharVector(info.bundleName);
    std::vector<char> abilityName = StringToCharVector(info.abilityName);
    std::vector<char> message = StringToCharVector(info.message);
    osInfo.deviceID = &deviceId[0];
    osInfo.bundleName = &bundleName[0];
    osInfo.abilityName = &abilityName[0];
    osInfo.message = &message[0];
    osInfo.messageLength = static_cast<uint32_t>(message.size());

    mutex.lock();
    if (callBacks.size() > 1) {
        ILOG("VirtualMessage callBacks size > 1.");
    }

    for (auto iter = callBacks.begin(); iter != callBacks.end(); iter++) {
        if (iter->second.first == nullptr) {
            ILOG("VirtualAbilityKit::VirtualMessage callback is null");
        }
        iter->second.first((reinterpret_cast<void*>(&info)));
    }
    mutex.unlock();
}
