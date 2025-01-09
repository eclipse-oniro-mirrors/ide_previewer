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
#include <dirent.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#elif __linux__
#include <climits>
#elif __APPLE__
#include <mach-o/dyld.h>
#endif


#include "PreviewerEngineLog.h"
#include "NativeFileSystem.h"

std::vector<std::string> FileSystem::pathList = {"file_system", "app", "ace", "data"};
std::string FileSystem::bundleName = "";
std::string FileSystem::fileSystemPath = "";

std::string FileSystem::separator = "/";

bool FileSystem::IsFileExists(std::string path)
{
    return S_ISREG(GetFileMode(path));
}

bool FileSystem::IsDirectoryExists(std::string path)
{
    return S_ISDIR(GetFileMode(path));
}

std::string FileSystem::GetApplicationPath()
{
    char appPath[MAX_PATH_LEN];
    if (getcwd(appPath, MAX_PATH_LEN) == nullptr) {
        ELOG("Get current path failed.");
        return std::string();
    }
    std::string path(appPath);
    return path;
}

const std::string& FileSystem::GetVirtualFileSystemPath()
{
    return fileSystemPath;
}

void FileSystem::MakeVirtualFileSystemPath()
{
    std::string dirToMake = GetApplicationPath();
    if (!IsDirectoryExists(dirToMake)) {
        ELOG("Application path is not exists.");
        return;
    }
    for (std::string path : pathList) {
        dirToMake += separator;
        dirToMake += path;
        MakeDir(dirToMake.data());
    }
    dirToMake += separator;
    dirToMake += bundleName;
    MakeDir(dirToMake);
    fileSystemPath = dirToMake;
}

int FileSystem::MakeDir(const std::string &path)
{
    std::string basePath = BaseDir(path);
    if (!basePath.empty() && !IsDirectoryExists(basePath)) {
        if (int err = MakeDir(basePath)) {
            return err;
        }
    }
    int result = 0;
#ifdef _WIN32
    result = mkdir(path.data());
#else
    result = mkdir(path.data(), S_IRUSR | S_IWUSR | S_IXUSR);
#endif
    return result;
}

std::string FileSystem::BaseDir(const std::string &path)
{
    return path.substr(0, path.rfind(separator));
}

std::string FileSystem::BaseName(const std::string &path)
{
    return path.substr(path.rfind(separator) + 1);
}

bool FileSystem::RemoveDir(const std::string &dirPath)
{
    DIR *dir = opendir(dirPath.c_str());
    if (dir == nullptr) {
        ELOG("Failed to open directory");
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string fileOrDirName = entry->d_name;
        if (fileOrDirName == "." || fileOrDirName == "..") {
            continue;
        }

        std::string fullPath = dirPath + separator + fileOrDirName;

        struct stat pathStat;
        if (stat(fullPath.c_str(), &pathStat) == -1) {
            ELOG(("Failed to get file status: " + fullPath).c_str());
            closedir(dir);
            return false;
        }

        if (S_ISDIR(pathStat.st_mode)) {
            if (!RemoveDir(fullPath)) {
                closedir(dir);
                return false;
            }
        } else {
            if (remove(fullPath.c_str()) != 0) {
                ELOG(("Failed to delete file: " + fullPath).c_str());
                closedir(dir);
                return false;
            }
        }
    }

    closedir(dir);

    if (rmdir(dirPath.c_str()) != 0) {
        ELOG(("Failed to remove directory: " + dirPath).c_str());
        return false;
    }

    return true;
}

static std::string GetTempDir()
{
#ifdef _WIN32
    char tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    return std::string(tempPath);
#else
    char* tempPath = getenv("TMPDIR");
    return tempPath == nullptr ? std::string("/tmp") : std::string(tempPath);
#endif
}

std::string FileSystem::CreateTempDirectory()
{
#ifdef _WIN32
    char tempDir[MAX_PATH];
    if (GetTempFileName(GetTempDir().c_str(), "previewer", 0, tempDir) == 0) {
        ELOG("Failed to create temporary directory");
        return "";
    }
    DeleteFile(tempDir);
    if (!CreateDirectory(tempDir, nullptr)) {
        ELOG("Failed to create temporary directory");
        return "";
    }
    return std::string(tempDir);
#else
    std::string tempDirTemplate = GetTempDir() + "/previewer_XXXXXX";
    char tempDir[tempDirTemplate.size() + 1];
    tempDirTemplate.copy(tempDir, tempDirTemplate.size());
    if (mkdtemp(tempDir) == nullptr) {
        ELOG("Failed to create temporary directory");
        return "";
    }
    return std::string(tempDir);
#endif
}

void FileSystem::SetBundleName(std::string name)
{
    bundleName = name;
}

unsigned short FileSystem::GetFileMode(std::string path)
{
    struct stat info {};
    if (stat(path.data(), &info) != 0) {
        return 0;
    }
    return info.st_mode;
}

std::string FileSystem::GetSeparator()
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

std::string FileSystem::GetFullPath(const std::string& path)
{
    char fullPath[PATH_MAX];

#ifdef _WIN32
    if (GetFullPathName(path.c_str(), PATH_MAX, fullPath, NULL)) {
        return std::string(fullPath);
    }
#elif __linux__ || __APPLE__
    if (realpath(path.c_str(), fullPath)) {
        return std::string(fullPath);
    }
#endif

    return path;
}

std::string FileSystem::GetExecutablePath()
{
    std::string path;

#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    path = std::string(buffer).substr(0, pos);

#elif __linux__
    char buffer[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
    if (count != -1) {
        path = std::string(buffer, (count > 0) ? count : 0);
        std::string::size_type pos = path.find_last_of('/');
        path = path.substr(0, pos);
    }

#elif __APPLE__
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        path = std::string(buffer);
        std::string::size_type pos = path.find_last_of('/');
        path = path.substr(0, pos);
    }
#endif

    ILOG("Executable path: %s", path.c_str());

    return path;
}
