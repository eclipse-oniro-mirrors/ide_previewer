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

#include "NativeFileSystem.h"
#include <locale>
#include <codecvt>
#include <windows.h>
#include "PreviewerEngineLog.h"

namespace OHOS::Ide {
std::string NativeFileSystem::FindSubfolderByName(const std::string& parentFolderPath,
    const std::string& subfolderName)
{
    WIN32_FIND_DATAW datas;
    std::wstring path = std::wstring(parentFolderPath.begin(), parentFolderPath.end());
    std::wstring searchPath = path + L"\\*";
    HANDLE handle = FindFirstFileW(searchPath.c_str(), &datas);
    if (handle == INVALID_HANDLE_VALUE) {
        ELOG("failed to open directory:%s.", parentFolderPath.c_str());
        return "";
    }
    do {
        if (datas.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            std::wstring name = datas.cFileName;
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::string dir = converter.to_bytes(name);
            if (dir.find(subfolderName) == 0) {
                FindClose(handle);
                std::string filePath = parentFolderPath + "\\" + dir;
                return filePath;
            }
        }
    } while (FindNextFileW(handle, &datas) != 0);
    FindClose(handle);
    return "";
}
}