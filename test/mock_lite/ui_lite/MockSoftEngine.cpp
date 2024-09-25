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

#include "soft_engine.h"
#include "MockGlobalResult.h"

namespace OHOS {
    // all functions only for mock test, no specific implementation
    void SoftEngine::DrawArc(BufferInfo& dst, ArcInfo& arcInfo, const Rect& mask, const Style& style,
        OpacityType opacity, uint8_t cap) {}

    void SoftEngine::DrawLine(BufferInfo& dst, const Point& start, const Point& end, const Rect& mask,
        int16_t width, ColorType color, OpacityType opacity) {}

    void SoftEngine::DrawLetter(BufferInfo& gfxDstBuffer, const uint8_t* fontMap, const Rect& fontRect,
        const Rect& subRect, const uint8_t fontWeight, const ColorType& color, const OpacityType opa) {}

    void SoftEngine::DrawCubicBezier(BufferInfo& dst, const Point& start, const Point& control1,
        const Point& control2, const Point& end, const Rect& mask, int16_t width, ColorType color,
        OpacityType opacity) {}

    void SoftEngine::DrawRect(BufferInfo& dst, const Rect& rect, const Rect& dirtyRect, const Style& style,
        OpacityType opacity) {}

    void SoftEngine::DrawTransform(BufferInfo& dst, const Rect& mask, const Point& position, ColorType color,
        OpacityType opacity, const TransformMap& transMap, const TransformDataInfo& dataInfo) {}

    void SoftEngine::ClipCircle(const ImageInfo* info, float x, float y, float radius) {}

    void SoftEngine::Blit(BufferInfo& dst, const Point& dstPos, const BufferInfo& src, const Rect& subRect,
        const BlendOption& blendOption) {}
    
    void SoftEngine::Fill(BufferInfo& dst, const Rect& fillArea, const ColorType color,
        const OpacityType opacity) {}
    
    void SoftEngine::DrawPath(BufferInfo& dst, void* param, const Paint& paint, const Rect& rect,
        const Rect& invalidatedArea, const Style& style) {}

    void SoftEngine::FillPath(BufferInfo& dst, void* param, const Paint& paint, const Rect& rect,
        const Rect& invalidatedArea, const Style& style) {}

    uint8_t* SoftEngine::AllocBuffer(uint32_t size, uint32_t usage)
    {
        return nullptr;
    }

    void SoftEngine::FreeBuffer(uint8_t* buffer, uint32_t usage) {}
}