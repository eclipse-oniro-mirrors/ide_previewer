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

#include "ui_font_vector.h"

namespace {
    int8_t defaultRetVal = 1;
}

namespace OHOS {
    UIFontVector::UIFontVector() {}

    UIFontVector::~UIFontVector() {}

    bool UIFontVector::IsColorEmojiFont(FT_Face& face)
    {
        return true;
    }

    int8_t SetupColorFont(FT_Face face, uint8_t fontSize)
    {
        return defaultRetVal;
    }

    uint8_t UIFontVector::RegisterFontInfo(const char* ttfName, uint8_t shaping)
    {
        return defaultRetVal;
    }

    uint8_t UIFontVector::RegisterFontInfo(const UITextLanguageFontParam* fontsTable, uint8_t num)
    {
        return defaultRetVal;
    }

    uint8_t UIFontVector::RegisterTtcFontInfo(const char* ttcName, const TtfInfo* ttfInfo, uint8_t count)
    {
        return defaultRetVal;
    }

    uint8_t UIFontVector::UnregisterTtcFontInfo(const char* ttcName, const TtfInfo* ttfInfo, uint8_t count)
    {
        return defaultRetVal;
    }

    uint8_t UIFontVector::UnregisterFontInfo(const UITextLanguageFontParam* fontsTable, uint8_t num)
    {
        return defaultRetVal;
    }

    uint8_t UIFontVector::UnregisterFontInfo(const char* ttfName)
    {
        return defaultRetVal;
    }

    const UITextLanguageFontParam* UIFontVector::GetFontInfo(uint16_t fontId) const
    {
        return nullptr;
    }

    int32_t UIFontVector::OpenVectorFont(uint8_t ttfId)
    {
        return defaultRetVal;
    }

    bool UIFontVector::GetTtfInfo(uint8_t ttfId, uint8_t* ttfBuffer, uint32_t ttfBufferSize, TtfHeader& ttfHeader)
    {
        return true;
    }

    bool UIFontVector::GetTtfInfoFromTtf(uint8_t* ttfBuffer, uint32_t ttfBufferSize, TtfHeader& ttfHeader,
        UITextLanguageFontParam fontInfo)
    {
        return true;
    }

    struct TtcHeader {
        uint32_t ttcTag;
        uint16_t major;
        uint16_t minor;
        int32_t numFonts;
    };

    bool UIFontVector::GetTtfInfoFromTtc(uint8_t* ttfBuffer, uint32_t ttfBufferSize, TtfHeader& ttfHeader,
        UITextLanguageFontParam fontInfo)
    {
        return true;
    }

    bool UIFontVector::IsVectorFont() const
    {
        return true;
    }

    uint8_t UIFontVector::GetFontWeight(uint16_t fontId)
    {
        return defaultRetVal;
    }

    int8_t UIFontVector::SetFontPath(const char* path, FontType type)
    {
        return defaultRetVal;
    }

    int8_t UIFontVector::GetFaceInfo(uint16_t fontId, uint8_t fontSize, FaceInfo& faceInfo)
    {
        return defaultRetVal;
    }

    uint16_t UIFontVector::GetHeight(uint16_t fontId, uint8_t fontSize)
    {
        return defaultRetVal;
    }

    uint16_t UIFontVector::GetShapingFontId(char* text, uint8_t& ttfId, uint32_t& script,
        uint16_t fontId, uint8_t size) const
    {
        return defaultRetVal;
    }

    uint16_t UIFontVector::GetFontId(const char* ttfName, uint8_t fontSize) const
    {
        return defaultRetVal;
    }

    uint16_t UIFontVector::GetFontId(uint32_t unicode) const
    {
        return defaultRetVal;
    }

    int16_t UIFontVector::GetWidth(uint32_t unicode, uint16_t fontId, uint8_t fontSize)
    {
        return defaultRetVal;
    }

    int8_t UIFontVector::GetFontHeader(FontHeader& fontHeader, uint16_t fontId, uint8_t fontSize)
    {
        return defaultRetVal;
    }

    void UIFontVector::SaveGlyphNode(uint32_t unicode, uint16_t fontKey, Metric *metric) {}

    int8_t UIFontVector::GetGlyphNode(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize)
    {
        return defaultRetVal;
    }

    uint8_t* UIFontVector::GetBitmap(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize)
    {
        return nullptr;
    }

    bool UIFontVector::IsEmojiFont(uint16_t fontId)
    {
        return true;
    }

    uint8_t UIFontVector::IsGlyphFont(uint32_t unicode)
    {
        return defaultRetVal;
    }

    void UIFontVector::SetFace(FaceInfo& faceInfo, uint32_t unicode) {}

    void UIFontVector::SetFace(FaceInfo& faceInfo, uint32_t unicode, TextStyle textStyle) {}

    void UIFontVector::ClearFontGlyph(FT_Face face) {}

    inline uint16_t UIFontVector::GetKey(uint16_t fontId, uint8_t size)
    {
        return defaultRetVal;
    }

    uint16_t UIFontVector::GetOffsetPosY(const char* text, uint16_t lineLength, bool& isEmojiLarge,
        uint16_t fontId, uint8_t fontSize)
    {
        return defaultRetVal;
    }

    uint16_t UIFontVector::GetLineMaxHeight(const char* text, uint16_t lineLength, uint16_t fontId,
        uint8_t fontSize, uint16_t& letterIndex, SpannableString* spannableString)
    {
        return defaultRetVal;
    }

    uint16_t UIFontVector::GetMaxSubLineHeight(uint16_t textNum, uint16_t loopNum, uint16_t maxHeight,
        uint16_t emojiNum)
    {
        return defaultRetVal;
    }

    void UIFontVector::SetPsramMemory(uintptr_t psramAddr, uint32_t psramLen) {}

    int8_t UIFontVector::SetCurrentLangId(uint8_t langId)
    {
        return defaultRetVal;
    }
} // namespace OHOS
