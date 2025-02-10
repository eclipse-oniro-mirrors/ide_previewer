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

#include "Unhap.h"

#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <fstream>

#include "PreviewerEngineLog.h"
#include "FileSystem.h"
#include "contrib/minizip/unzip.h"
#include "zlib.h"

using namespace std;

bool SetupPandaClassPath(const string &destDir)
{
    constexpr uint8_t extFileSize = 3;
    string pandaClassPath = FileSystem::NormalizePath(destDir + "/libs/x86-64/");
    if (!FileSystem::IsDirectoryExists(pandaClassPath) || !FileSystem::IsDirectoryExists("./module/")) {
        return true;
    }
#ifdef _WIN32
    if (getenv("PANDA_CLASS_PATH") == nullptr) {
        string env = string("PANDA_CLASS_PATH=") + pandaClassPath;
        putenv(env.data());
    }
#elif __linux__ || __APPLE__
    setenv("PANDA_CLASS_PATH", pandaClassPath.c_str(), 0);
#endif
    DIR *hPandaClassDir = opendir(pandaClassPath.c_str());
    if (!hPandaClassDir) {
        return true;
    }
    struct dirent *dir;
    while ((dir = readdir(hPandaClassDir)) != nullptr) {
        // Rename .abc.so files to .abc because Panda VM could not start .abc.so
        string fileNameSrc = dir->d_name;
        if (fileNameSrc.find(".abc.so") != string::npos) {
            string fileNameDst = fileNameSrc;
            fileNameDst.replace(fileNameDst.end() - extFileSize, fileNameDst.end(), "");
            rename((pandaClassPath + fileNameSrc).c_str(), (pandaClassPath + fileNameDst).c_str());
            continue;
        }
        // Copy native libs to "./module" folder of Previewer
        if (fileNameSrc.find(".so") != string::npos) {
            ifstream src(pandaClassPath + fileNameSrc, ios::binary);
            string fileNameDst = "./module/" + FileSystem::BaseName(dir->d_name);
            ofstream dst(fileNameDst, ios::binary);
            dst << src.rdbuf();
            src.close();
            dst.close();
        }
    }
    closedir(hPandaClassDir);
    return true;
}

bool Unhap(const string &hapPath, const string &destDir)
{
    unzFile zipfile = unzOpen(hapPath.c_str());
    if (zipfile == nullptr) {
        ELOG("Failed to open ZIP archive");
        return false;
    }
    // Iterate through all files in the archive
    if (unzGoToFirstFile(zipfile) != UNZ_OK) {
        unzClose(zipfile);
        ELOG("Error while going to the first file");
        return false;
    }
    do {
        char filename[FILENAME_MAX];
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipfile, &fileInfo, filename, sizeof(filename), nullptr, 0, nullptr, 0) != UNZ_OK) {
            unzClose(zipfile);
            ELOG("Error while getting file information");
            return false;
        }
        string fullPath = FileSystem::NormalizePath(destDir + "/" + filename);
        // Check if it is a directory
        if (fullPath.back() == FileSystem::GetSeparator()[0]) {
            FileSystem::MakeDir(fullPath);
        } else {
            FileSystem::MakeDir(FileSystem::BaseDir(fullPath));
            // It's a file, extract it
            if (unzOpenCurrentFile(zipfile) != UNZ_OK) {
                unzClose(zipfile);
                ELOG("Failed to open file inside the archive");
                return false;
            }
            FILE *f = fopen(fullPath.c_str(), "wb");
            if (!f) {
                unzCloseCurrentFile(zipfile);
                unzClose(zipfile);
                ELOG("Failed to create file for writing");
                return false;
            }
            char buffer[2 * FILENAME_MAX];
            int bytesRead;
            while ((bytesRead = unzReadCurrentFile(zipfile, buffer, sizeof(buffer))) > 0) {
                fwrite(buffer, 1, bytesRead, f);
            }
            fclose(f);
            unzCloseCurrentFile(zipfile);
        }
    } while (unzGoToNextFile(zipfile) == UNZ_OK);
    unzClose(zipfile);
    return SetupPandaClassPath(destDir);
}
