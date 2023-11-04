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
#include <dirent.h>
#include <sys/stat.h>
#include "PreviewerEngineLog.h"

namespace OHOS::Ide {
std::string NativeFileSystem::FindSubfolderByName(const std::string& parentFolderPath,
    const std::string& subfolderName)
{
    DIR* dir = opendir(parentFolderPath.c_str());
    if (dir == nullptr) {
        ELOG("failed to open directory:%s.", parentFolderPath.c_str());
        return "";
    }
    struct dirent* dirEntry;
    while ((dirEntry = readdir(dir)) != nullptr) {
        struct stat entryStat;
        std::string filePath = parentFolderPath + "/" + dirEntry->d_name;
        if (stat(filePath.c_str(), &entryStat) != -1 && S_ISDIR(entryStat.st_mode)) {
            std::string dirName(dirEntry->d_name);
            if (dirName.find(subfolderName) == 0) {
                closedir(dir);
                return filePath;
            }
        }
    }
    closedir(dir);
    return "";
}
}