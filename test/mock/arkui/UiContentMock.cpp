/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "UiContentMock.h"
#include "MockGlobalResult.h"

namespace OHOS::Ace {
std::string UIContent::GetJSONTree()
{
    g_uiContentGetJSonTree = true;
    return "uicontentTree";
}

void UIContent::LoadDocument(const std::string& url, const std::string& componentName,
    const Platform::SystemParams& params)
{
    g_uiContentLoadDocument = true;
}

bool UIContent::OperateComponent(const std::string& attrsJson)
{
    g_uiContentOperateComponent = true;
    return true;
}
}