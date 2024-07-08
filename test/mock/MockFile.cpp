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

#include "MockFile.h"
#include <filesystem>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include "zlib.h"
#include "contrib/minizip/zip.h"
using namespace testmock;
namespace fs = std::filesystem;

bool MockFile::SimulateFileLock(const std::string& filePath)
{
    int fd = open(filePath.c_str(), O_RDWR | O_CREAT);
    if (fd == -1) {
        // Handle error
        return false;
    }

    // Attempt to acquire an exclusive lock
    struct flock fl;
    fl.l_type = F_WRLCK;    // Exclusive write lock
    fl.l_whence = SEEK_SET; // Starting from beginning of file
    fl.l_start = 0;         // Starting from offset 0
    fl.l_len = 0;           // Lock whole file
    fl.l_pid = getpid();    // PID of process holding the lock

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        // Lock failed (file is already locked)
        close(fd);
        return true; // Simulate that file is locked
    }

    // Lock acquired, simulate that file is not locked
    close(fd);
    return false;
}

bool MockFile::ReleaseFileLock(const std::string& filePath)
{
    int fd = open(filePath.c_str(), O_RDWR | O_CREAT);
    if (fd == -1) {
        // Handle error
        return false;
    }

    struct flock fl;
    fl.l_type = F_UNLCK;    // Unlock the file
    fl.l_whence = SEEK_SET; // Starting from beginning of file
    fl.l_start = 0;         // Starting from offset 0
    fl.l_len = 0;           // Unlock whole file

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        // Failed to unlock
        close(fd);
        return false;
    }

    // Successfully unlocked
    close(fd);
    return true;
}

// Function to add a file to zip
bool MockFile::AddFileToZip(zipFile zip, const std::string& filePath, const std::string& entryName)
{
    if (zipOpenNewFileInZip(zip, entryName.c_str(), NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED,
        Z_DEFAULT_COMPRESSION) != ZIP_OK) {
        std::cerr << "Failed to create entry in zip file for " << filePath << std::endl;
        return false;
    }

    FILE* file = fopen(filePath.c_str(), "rb");
    if (!file) {
        std::cerr << "Failed to open file " << filePath << std::endl;
        zipCloseFileInZip(zip);
        return false;
    }

    const int bufferSize = 1024;
    void* buffer = malloc(bufferSize);
    int size;
    while ((size = fread(buffer, 1, bufferSize, file)) > 0) {
        if (zipWriteInFileInZip(zip, buffer, size) < 0) {
            std::cerr << "Failed to write to zip for " << filePath << std::endl;
            free(buffer);
            if (fclose(file) == EOF) {
                std::cerr << "Failed to close file" << std::endl;
                return false;
            }
            zipCloseFileInZip(zip);
            return false;
        }
    }

    free(buffer);
    if (fclose(file) == EOF) {
        std::cerr << "Failed to close file" << std::endl;
        return false;
    }
    zipCloseFileInZip(zip);

    return true;
}

// Function to add a folder and its contents recursively to zip
bool MockFile::AddFolderToZip(zipFile zip, const std::string& folderPath, const std::string& entryName)
{
    for (const auto& entry : fs::recursive_directory_iterator(folderPath)) {
        std::string relativePath = entry.path().string().substr(folderPath.length() + 1); // Relative path

        if (fs::is_directory(entry)) {
            // Create directory entry
            if (zipOpenNewFileInZip(zip, (entryName + "/" + relativePath + "/").c_str(), NULL, NULL, 0,
                NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK) {
                std::cerr << "Failed to create entry in zip file for " << entry.path() << std::endl;
                return false;
            }
            zipCloseFileInZip(zip);
        } else {
            // Add file
            if (!AddFileToZip(zip, entry.path().string(), entryName + "/" + relativePath)) {
                return false;
            }
        }
    }

    return true;
}

// Main compression function
bool MockFile::CompressFiles(const std::vector<std::string>& files, const std::string& zipFilename)
{
    zipFile zip = zipOpen(zipFilename.c_str(), APPEND_STATUS_CREATE);
    if (!zip) {
        std::cerr << "Could not create zip file " << zipFilename << std::endl;
        return false;
    }

    for (const auto& file : files) {
        if (fs::is_directory(file)) {
            // Add directory and its contents
            if (!AddFolderToZip(zip, file, fs::path(file).filename())) {
                zipClose(zip, NULL);
                return false;
            }
        } else {
            // Add individual file
            if (!AddFileToZip(zip, file, fs::path(file).filename())) {
                zipClose(zip, NULL);
                return false;
            }
        }
    }

    if (zipClose(zip, NULL) != ZIP_OK) {
        std::cerr << "Failed to close zip file " << zipFilename << std::endl;
        return false;
    }

    return true;
}

// 将内容写入 module.json 文件
void WriteToFile(const std::string& filePath, const std::string& content)
{
    std::ofstream file(filePath);
    if (!file) {
        std::cerr << "Error creating file: " << filePath << std::endl;
        return;
    }
    file << content;
    file.close();
}

void CreateFiles(const std::string hspAbcContent)
{
    std::filesystem::path dir("ets");

    // 检查文件夹是否存在
    if (std::filesystem::exists(dir)) {
        std::cout << "Folder already exists." << std::endl;
    } else {
        // 创建文件夹
        std::filesystem::create_directory(dir);
        std::cout << "Folder created successfully." << std::endl;
    }
    // 生成hsp文件
    // 在ets下写入文件modules.abc
    WriteToFile("ets/modules.abc", hspAbcContent);
    // 在当前目录下写入文件module.json
    WriteToFile("module.json", hspAbcContent);
}

std::string MockFile::CreateHspFile(const std::string hspFileName, const std::string hspAbcContent)
{
    CreateFiles(hspAbcContent);
    std::vector<std::string> filesToCompress = { "ets", "module.json" };
    std::string zipFilename = hspFileName + ".zip";
    std::string newFileName = hspFileName + ".hsp";
    if (CompressFiles(filesToCompress, zipFilename)) {
        std::cout << "Compression successful. File created: " << zipFilename << std::endl;
        if (std::rename(zipFilename.c_str(), newFileName.c_str()) != 0) {
            std::cout << newFileName << " 创建hsp文件失败" << std::endl;
            return "";
        }
    } else {
        std::cerr << "Compression failed." << std::endl;
        return "";
    }
    return newFileName;
}
