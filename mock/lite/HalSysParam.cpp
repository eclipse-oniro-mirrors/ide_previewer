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

#include <cstring>

#include "hal_sys_param.h"

#include "FileSystem.h"
#include "ModelManager.h"
#include "PreviewerEngineLog.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static char g_mockString[2] = "";

const char* HalGetDeviceType(void)
{
    ILOG("Get productType, result: %s", ModelManager::GetConfig().deviceType.c_str());
    if (ModelManager::GetConfig().deviceType.empty()) {
        ILOG("GetDeviceType is null.");
        return nullptr;
    }
    return ModelManager::GetConfig().deviceType.c_str();
}

const char* HalGetManufacture(void)
{
    ILOG("Get manufacture, result: %s", ModelManager::GetConfig().manufactureName.c_str());
    if (ModelManager::GetConfig().manufactureName.empty()) {
        ILOG("GetManufacture is null.");
        return nullptr;
    }
    return ModelManager::GetConfig().manufactureName.c_str();
}

const char* HalGetBrand(void)
{
    ILOG("Get brand, result: %s", ModelManager::GetConfig().brandName.c_str());
    if (ModelManager::GetConfig().brandName.empty()) {
        ILOG("GetBrand is null.");
        return nullptr;
    }
    return ModelManager::GetConfig().brandName.c_str();
}

const char* HalGetMarketName(void)
{
    return g_mockString;
}

const char* HalGetProductSeries(void)
{
    return g_mockString;
}

const char* HalGetProductModel(void)
{
    ILOG("Get model, result: %s", ModelManager::GetConfig().modelName.c_str());
    if (ModelManager::GetConfig().modelName.empty()) {
        ILOG("GetProductModel is null.");
        return nullptr;
    }
    return ModelManager::GetConfig().modelName.c_str();
}

const char* HalGetSoftwareModel(void)
{
    return g_mockString;
}

const char* HalGetHardwareModel(void)
{
    return g_mockString;
}

const char* HalGetHardwareProfile(void)
{
    return g_mockString;
}

const char* HalGetSerial(void)
{
    return g_mockString;
}

const char* HalGetBootloaderVersion(void)
{
    return g_mockString;
}

const char* HalGetAbiList(void)
{
    return g_mockString;
}

const char* HalGetDisplayVersion(void)
{
    return g_mockString;
}

const char* HalGetIncrementalVersion(void)
{
    return g_mockString;
}

const char* HalGetBuildType(void)
{
    return g_mockString;
}

const char* HalGetBuildUser(void)
{
    return g_mockString;
}

const char* HalGetBuildHost(void)
{
    return g_mockString;
}

const char* HalGetBuildTime(void)
{
    return g_mockString;
}

int HalGetFirstApiVersion(void)
{
    return 0;
}

const char* GetDataPath()
{
    return FileSystem::GetVirtualFileSystemPath().data();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
