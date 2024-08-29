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

#include "StringHelper.h"
#include <windows.h>
#include "securec.h"
#include "PreviewerEngineLog.h"

std::string StringHelper::StringToUtf8(const std::string& str)
{
    int doubles = 2;
    int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    if (nwLen < 1) {
        ELOG("MultiByteToWideChar failed.");
        return str;
    }
    wchar_t* pwBuf = new(std::nothrow) wchar_t[nwLen + 1];
    if (!pwBuf) {
        ELOG("Memory allocation failed : pwBuf.");
        return str;
    }
    ZeroMemory(pwBuf, (nwLen + 1) * doubles);
    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
    if (nLen < 1) {
        delete[] pwBuf;
        ELOG("WideCharToMultiByte failed.");
        return str;
    }
    char* pBuf = new(std::nothrow) char[nLen + 1];
    if (!pBuf) {
        ELOG("Memory allocation failed : pBuf.");
        return str;
    }
    ZeroMemory(pBuf, nLen + 1);
    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
    std::string retStr(pBuf);
    delete[] pwBuf;
    delete[] pBuf;
    pwBuf = NULL;
    pBuf = NULL;
    return retStr;
}

std::string StringHelper::Utf8ToString(const std::string& str)
{
    int doubles = 2;
    int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    int wLenAdd = nwLen + 1;
    if (wLenAdd <= 1) {
        return str;
    }
    wchar_t* pwBuf = new(std::nothrow) wchar_t[wLenAdd];
    if (!pwBuf) {
        ELOG("Memory allocation failed : pwBuf.");
        return str;
    }
    int doubleLen = wLenAdd * doubles;
    if (EOK != memset_s(pwBuf, sizeof(*pwBuf) * doubles, 0, doubleLen)) {
        delete []pwBuf;
        ELOG("pwBuf memset_s failed.");
        return str;
    }
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
    int lenAdd = nLen + 1;
    if (lenAdd <= 1) {
        delete []pwBuf;
        return str;
    }
    char* pBuf = new(std::nothrow) char[lenAdd];
    if (!pBuf) {
        ELOG("Memory allocation failed : pBuf.");
        return str;
    }
    if (EOK != memset_s(pBuf, lenAdd, 0, lenAdd)) {
        delete []pBuf;
        ELOG("pBuf memset_s failed.");
        return str;
    }
    WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
    std::string retStr = pBuf;
    delete []pBuf;
    delete []pwBuf;
    pBuf = NULL;
    pwBuf = NULL;
    return retStr;
}