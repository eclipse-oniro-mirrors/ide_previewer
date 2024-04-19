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

#include "glfw_render_context.h"

namespace OHOS::Rosen {
std::shared_ptr<GlfwRenderContext> GlfwRenderContext::GetGlobal()
{
    if (!global_) {
        global_ = std::make_shared<GlfwRenderContext>();
    }
    return global_;
}

int GlfwRenderContext::Init()
{
    return 1;
}

int GlfwRenderContext::CreateGlfwWindow(int32_t width, int32_t height, bool visible)
{
    return 1;
}

void GlfwRenderContext::DestroyWindow() {}

void GlfwRenderContext::Terminate() {}

void GlfwRenderContext::PollEvents() {}

void GlfwRenderContext::SetWindowSize(int32_t width, int32_t height)
{
    width_ = width;
    height_ = height;
}
}