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

#ifndef STAGE_CONTEXT_H
#define STAGE_CONTEXT_H

#include <string>
#include <vector>
#include <optional>
#include <map>

namespace Json {
    class Value;
}

namespace OHOS::Ide {
class StageContext {
public:
    static StageContext& GetInstance();
    const std::optional<std::vector<uint8_t>> ReadFileContents(const std::string& filePath) const;
    // for Previewer
    void SetLoaderJsonPath(const std::string& assetPath);
    void GetModulePathMapFromLoaderJson();
    void ReleaseHspBuffers();
    // for ArkUI and Ability
    std::vector<uint8_t>* GetModuleBuffer(const std::string& inputPath);
private:
    StageContext() = default;
    ~StageContext() = default;
    bool ContainsRelativePath(const std::string& path) const;
    std::map<std::string, std::string> GetModulePathMap() const;
    std::string loaderJsonPath;
    std::map<std::string, std::string> modulePathMap;
    std::vector<std::vector<uint8_t>*> hspBufferPtrsVec;
};
}
#endif // STAGE_CONTEXT_H