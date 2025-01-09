/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "adapter/preview/entrance/samples/event_adapter.h"

#include <map>
#include <memory>

namespace OHOS::Ace::Sample {

EventAdapter::EventAdapter() {}

EventAdapter::~EventAdapter() = default;

void EventAdapter::Initialize(std::shared_ptr<GlfwRenderContext>& glfwRenderContext) {}

void EventAdapter::RegisterKeyEventCallback(MMIKeyEventCallback&& callback) {}

void EventAdapter::RegisterPointerEventCallback(MMIPointerEventCallback&& callback) {}

void EventAdapter::RegisterInspectorCallback(InspectorCallback&& callback) {}

bool EventAdapter::RecognizeKeyEvent(int key, int action, int mods)
{
    return true;
}

void EventAdapter::RecognizePointerEvent(const TouchType type) {}

bool EventAdapter::RunSpecialOperations(int key, int action, int mods)
{
    return true;
}

} // namespace OHOS::Ace::Sample
