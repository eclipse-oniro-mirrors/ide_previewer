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

#include "FileSystem.h"

#include <sys/stat.h>
#include <unistd.h>

#include "PreviewerEngineLog.h"
#include "NativeFileSystem.h"
using namespace std;

vector<string> FileSystem::pathList = {"file_system", "app", "ace", "data"};
string FileSystem::bundleName = "";
string FileSystem::fileSystemPath = "";

#ifdef _WIN32
std::string FileSystem::separator = "\\";
#else
std::string FileSystem::separator = "/";
#endif

bool FileSystem::IsFileExists(string path)
{
    return S_ISREG(GetFileMode(path));
}

bool FileSystem::IsDirectoryExists(string path)
{
    return S_ISDIR(GetFileMode(path));
}

string FileSystem::GetApplicationPath()
{
    char appPath[MAX_PATH_LEN];
    if (getcwd(appPath, MAX_PATH_LEN) == nullptr) {
        ELOG("Get current path failed.");
        return string();
    }
    string path(appPath);
    return path;
}

const string& FileSystem::GetVirtualFileSystemPath()
{
    return fileSystemPath;
}

void FileSystem::MakeVirtualFileSystemPath()
{
    string dirToMake = GetApplicationPath();
    if (!IsDirectoryExists(dirToMake)) {
        ELOG("Application path is not exists.");
        return;
    }
    for (string path : pathList) {
        dirToMake += separator;
        dirToMake += path;
        MakeDir(dirToMake.data());
    }
    dirToMake += separator;
    dirToMake += bundleName;
    MakeDir(dirToMake);
    fileSystemPath = dirToMake;
}

int FileSystem::MakeDir(string path)
{
    int result = 0;
#ifdef _WIN32
    result = mkdir(path.data());
#else
    result = mkdir(path.data(), S_IRUSR | S_IWUSR | S_IXUSR);
#endif
    return result;
}

void FileSystem::SetBundleName(string name)
{
    bundleName = name;
}

unsigned short FileSystem::GetFileMode(string path)
{
    struct stat info {};
    if (stat(path.data(), &info) != 0) {
        return 0;
    }
    return info.st_mode;
}

string FileSystem::GetSeparator()
{
    return separator;
}

std::string FileSystem::FindSubfolderByName(const std::string& parentFolderPath, const std::string& subfolderName)
{
    return OHOS::Ide::NativeFileSystem::FindSubfolderByName(parentFolderPath, subfolderName);
}

std::string FileSystem::NormalizePath(const std::string& path)
{
    std::string normalizedPath = path;
    char separatorChar = FileSystem::separator[0]; // 0 is get fist char of string
    for (char& c : normalizedPath) {
        if (c == '/' || c == '\\') {
            c = separatorChar;
        }
    }
    return normalizedPath;
}