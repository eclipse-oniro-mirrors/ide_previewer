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

#include "ui_font.h"
#include "ui_font_vector.h"
#include "MockGlobalResult.h"

namespace {
    int8_t defaultRetVal = 1;
}

namespace OHOS {
    void BaseFont::SetPsramMemory(uintptr_t psramAddr, uint32_t psramLen)
    {
        g_uiFontSetPsramMemory = true;
    }

    UIFont::UIFont() : instance_(nullptr), defaultInstance_(nullptr) {}

    UIFont::~UIFont()
    {
        if (defaultInstance_) {
            delete defaultInstance_;
        }
    }

    UIFont* UIFont::GetInstance()
    {
        static UIFont instance;
        if (instance.instance_ == nullptr) {
            instance.defaultInstance_ = new UIFontVector();
            instance.instance_ = instance.defaultInstance_;
        }
        return &instance;
    }

    void UIFont::SetFont(BaseFont* font)
    {
        if (defaultInstance_ != nullptr) {
            delete defaultInstance_;
            defaultInstance_ = nullptr;
        }
        defaultInstance_ = font;
        instance_ = defaultInstance_;
        g_uiFontSetFont = true;
    }

    int8_t UIFont::SetFontPath(const char* path, BaseFont::FontType type)
    {
        return defaultRetVal;
    }

    int8_t UIFont::SetCurrentLangId(uint8_t langId)
    {
        return defaultRetVal;
    }

    uint16_t UIFont::GetLineMaxHeight(const char* text, uint16_t lineLength, uint16_t fontId, uint8_t fontSize,
        uint16_t letterIndex, SpannableString* spannableString)
    {
        return 0;
    }
}