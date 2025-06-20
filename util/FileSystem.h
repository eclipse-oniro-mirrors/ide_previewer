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

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>

class FileSystem {
public:
    static bool IsFileExists(std::string path);
    static bool IsDirectoryExists(std::string path);
    static std::string GetApplicationPath();
    static const std::string& GetVirtualFileSystemPath();
    static void MakeVirtualFileSystemPath();
    static int MakeDir(const std::string &path);
    static std::string BaseDir(const std::string &path);
    static std::string BaseName(const std::string &path);
    static bool RemoveDir(const std::string &dirPath);
    static std::string CreateTempDirectory();
    static void SetBundleName(std::string name);
    static std::string GetSeparator();
    static std::string FindSubfolderByName(const std::string& parentFolderPath, const std::string& subfolderName);
    static std::string NormalizePath(const std::string& path);
    static std::string GetFullPath(const std::string &path);
    static std::string GetExecutablePath();
private:
    static std::vector<std::string> pathList;
    static std::string separator;
    static std::string bundleName;
    static std::string fileSystemPath;
    static unsigned short GetFileMode(std::string path);
    static const uint32_t MAX_PATH_LEN = 260;
};

#endif // FILESYSTEM_H
