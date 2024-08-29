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

#include "ClipboardHelper.h"
#include <string>
#include <windows.h>
#include "StringHelper.h"

void ClipboardHelper::SetClipboardData(const std::string& data)
{
    std::string newData = StringHelper::Utf8ToString(data);
    OpenClipboard(NULL);
    EmptyClipboard();
    HANDLE hHandle = GlobalAlloc(GMEM_FIXED, newData.size() + 1);
    char* pData = (char*)GlobalLock(hHandle);
    copy(newData.begin(), newData.end(), pData);
    ::SetClipboardData(CF_TEXT, hHandle);
    GlobalUnlock(hHandle);
    CloseClipboard();
}

const std::string ClipboardHelper::GetClipboardData()
{
    std::string data;
    if (!IsClipboardFormatAvailable(CF_TEXT)) {
        return data;
    }
    OpenClipboard(NULL);
    HANDLE h = ::GetClipboardData(CF_TEXT);
    char* p = (char*)GlobalLock(h);
    std::string str(p);
    data = str;
    GlobalUnlock(h);
    CloseClipboard();
    StringHelper::Encode encode = StringHelper::DetectEncode((uint8_t*)str.c_str(), str.size());
    if (encode == StringHelper::Encode::ANSI) {
        data = StringHelper::StringToUtf8(data);
    }
    return data;
}