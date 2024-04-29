/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "VirtualScreenImpl.h"

using namespace std;

VirtualScreenImpl::~VirtualScreenImpl() {}

VirtualScreenImpl::VirtualScreenImpl() {}

VirtualScreenImpl& VirtualScreenImpl::GetInstance()
{
    static VirtualScreenImpl virtualScreen;
    return virtualScreen;
}

void VirtualScreenImpl::InitFlushEmptyTime()
{
    // judge loadDocTimeStamp
}

void VirtualScreenImpl::InitAll(string pipeName, string pipePort) {}

bool VirtualScreenImpl::LoadContentCallback(const std::string currentRouterPath)
{
    return true;
}

bool VirtualScreenImpl::Callback(const void* data, const size_t length,
    const int32_t width, const int32_t height, const uint64_t timeStamp)
{
    return true;
}

bool VirtualScreenImpl::PageCallback(const std::string currentRouterPath)
{
    return true;
}

void VirtualScreenImpl::FastPreviewCallback(const std::string& jsonStr) {}
