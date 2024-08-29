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

#include "TimerTaskHandler.h"

#if defined(LITEWEARABLE_SUPPORTED) && LITEWEARABLE_SUPPORTED
#include "CallbackManager.h"
#endif

#include "font/ui_font.h"
#include "global.h"
#include "JsonReader.h"
#include "task_manager.h"

#include "AsyncWorkManager.h"
#include "CommandLineInterface.h"
#include "JsAppImpl.h"
#include "LanguageManagerImpl.h"
#include "PreviewerEngineLog.h"
#include "SharedDataManager.h"

#if defined(LITEWEARABLE_SUPPORTED) && LITEWEARABLE_SUPPORTED
#include "VirtualLocation.h"
#endif

#include "VirtualScreenImpl.h"

using namespace OHOS::ACELite;

void TimerTaskHandler::TaskHandle()
{
    OHOS::TaskManager::GetInstance()->TaskHandler();
    VirtualScreenImpl::CheckBufferSend();
}

void TimerTaskHandler::CheckDevice()
{
    SharedDataManager::CheckTick();

#if defined(LITEWEARABLE_SUPPORTED) && LITEWEARABLE_SUPPORTED
    if (VirtualLocation::GetInstance().IsPostionChanged()) {
        VirtualLocation::GetInstance().ExecCallBack();
    }
#endif

    AsyncWorkManager::GetInstance().ExecAllAsyncWork();
}

#if defined(LITEWEARABLE_SUPPORTED) && LITEWEARABLE_SUPPORTED
void TimerTaskHandler::CheckBarometerChanged(uint32_t value)
{
    ILOG("CheckBarometerChanged value: %d", value);
    CallbackManager::GetInstance().ExecuteNumberInfoSuccess(SIMULATOR_JS_ATAPTER_SUBSCRIBE_BAROMETER, "pressure",
                                                            value);
}

void TimerTaskHandler::CheckStepCountChanged(uint32_t value)
{
    ILOG("CheckStepCountChanged value: %d", value);
    CallbackManager::GetInstance().ExecuteNumberInfoSuccess(SIMULATOR_JS_ATAPTER_SUBSCRIBE_STEP_COUNTER, "steps",
                                                            value);
}

void TimerTaskHandler::CheckHeartRateChanged(uint8_t value)
{
    ILOG("CheckHeartRateChanged value: %d", value);
    CallbackManager::GetInstance().ExecuteNumberInfoSuccess(SIMULATOR_JS_ATAPTER_SUBSCRIBE_HEART_RATE, "heartRate",
                                                            value);
}

void TimerTaskHandler::CheckOnBodyStateChanged(bool value)
{
    ILOG("CheckOnBodyStateChanged value: %d", value);
    CallbackManager::GetInstance().ExecuteBooleanInfoSuccess(SIMULATOR_JS_ATAPTER_SUBSCRIBE_ON_BODY_STATE, "value",
                                                             value);
}
#endif

void TimerTaskHandler::CheckLanguageChanged(std::string language)
{
    ILOG("CheckLanguageChanged value: %s", language.c_str());
    GLOBAL_ConfigLanguage(language.data());
}

void TimerTaskHandler::CheckBrightnessValueChanged(uint8_t value)
{
    Json2::Value result = JsonReader::CreateObject();
    result.Add("version", CommandLineInterface::COMMAND_VERSION.c_str());
    result.Add("property", "Brightness");
    Json2::Value brightness = JsonReader::CreateObject();
    brightness.Add("Brightness", static_cast<double>(value));
    result.Add("result", brightness);
    ILOG("CheckBrightnessValueChanged send Brightness: %d", static_cast<uint32_t>(value));
    CommandLineInterface::SendJsonData(result);
}

void TimerTaskHandler::CheckJsRunning()
{
    JsAppImpl::GetInstance().SetRunning(true);
}
