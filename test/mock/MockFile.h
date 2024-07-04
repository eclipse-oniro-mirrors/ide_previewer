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

#ifndef MOCKFILE_H
#define MOCKFILE_H

#include <string>
#include <vector>

typedef void* zipFile;

namespace testmock {
    class MockFile {
    public:
        static bool SimulateFileLock(const std::string& filePath);
        static bool ReleaseFileLock(const std::string& filePath);
        static std::string CreateHspFile(const std::string hspFileName, const std::string hspAbcContent);
        static bool AddFileToZip(zipFile zip, const std::string& filePath,
            const std::string& entryName);
        static bool AddFolderToZip(zipFile zip, const std::string& folderPath,
            const std::string& entryName);
        static bool CompressFiles(const std::vector<std::string>& files,
            const std::string& zipFilename);
    };
}

#endif // MOCKFILE_H
