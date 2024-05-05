#pragma once

#include "gapi/Limits.hpp"
#include "common/EnumClassOperators.hpp"

namespace RR::GAPI
{
    enum class FillMode : uint8_t
    {
        Wireframe,
        Solid
    }; // 1 bit

    enum class CullMode : uint8_t
    {
        None, // No culling
        Front, // Cull front-facing primitives
        Back, // Cull back-facing primitives
    }; // 2 bits

    enum class BlendFunc : uint8_t
    {
        Zero, // (0, 0, 0, 0)
        One, // (1, 1, 1, 1)
        SrcColor, // The fragment-shader output color
        OneMinusSrcColor, // One minus the fragment-shader output color
        DstColor, // The render-target color
        OneMinusDstColor, // One minus the render-target color
        SrcAlpha, // The fragment-shader output alpha value
        OneMinusSrcAlpha, // One minus the fragment-shader output alpha value
        DstAlpha, // The render-target alpha value
        OneMinusDstAlpha, // One minus the render-target alpha value
        SrcAlphaSaturate, // (f, f, f, 1), where f = min(fragment shader output alpha, 1 - render-target pixel alpha)
        Src1Color, // Fragment-shader output color 1
        OneMinusSrc1Color, // One minus fragment-shader output color 1
        Src1Alpha, // Fragment-shader output alpha 1
        OneMinusSrc1Alpha, // One minus fragment-shader output alpha 1
    }; // 4 bits

    enum class BlendOp : uint8_t
    {
        Add = 0, // Add src1 and src2
        Subtract = 1, // Subtract src1 from src2
        ReverseSubtract = 2, // Subtract src2 from src1
        Min = 3, // Find the minimum between the sources (per-channel)
        Max = 4, // Find the maximum between the sources (per-channel)
    }; // 3 bits

    enum class WriteMask : uint8_t
    {
        None = 0,
        Red = (1L << 0),
        Green = (1L << 1),
        Blue = (1L << 2),
        Alpha = (1L << 3),
        All = Red | Green | Blue | Alpha,
        RGB = Red | Green | Blue,
    }; // 4 bist
    ENUM_CLASS_BITWISE_OPS(WriteMask)

    struct RTBlendStateDesc
    {
        bool blendEnabled : 1;
        BlendOp rgbBlendOp : 3;
        BlendFunc srcRgbFunc : 4;
        BlendFunc dstRgbFunc : 4;
        BlendOp alphaBlendOp : 3;
        BlendFunc srcAlphaFunc : 4;
        BlendFunc dstAlphaFunc : 4;
        WriteMask writeMask : 4;
    }; // 4 bytes

    struct RasterizerDesc
    {
        FillMode fillMode : 1;
        CullMode cullMode : 2;
        bool isFrontCcw : 1;
        bool depthClipEnabled : 1;
        bool multisampleEnabled : 1;
        bool linesAAEnabled : 1;
        bool conservativeRasterEnabled : 1;
        float slopeScaledDepthBias;
        float depthBias;
        uint32_t forcedSampleCount;
    };
    
    struct BlendDesc
    {
        bool independentBlendEnabled : 1;
        bool alphaToCoverageEnabled : 1;
        std::array<RTBlendStateDesc, MAX_RENDER_TARGETS_COUNT> rtBlend;
    };
}