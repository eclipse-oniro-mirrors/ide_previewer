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

#include "js_async_work.h"

#include "AsyncWorkManager.h"
#include "PreviewerEngineLog.h"

using namespace OHOS::ACELite;

bool JsAsyncWork::DispatchAsyncWork(AsyncWorkHandler workHandler, void* data)
{
    if (workHandler == nullptr || data == nullptr) {
        ELOG("Error: workHandler or data is null.");
        return false;
    }
    AsyncWorkManager::GetInstance().AppendAsyncWork(workHandler, data);
    return true;
}
