#pragma once

#include "common/EnumClassOperators.hpp"
#include "gapi/Limits.hpp"
#include "gapi/Framebuffer.hpp"

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

    enum class BlendFactor : uint8_t
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
        RTBlendStateDesc() : blendEnabled(false),
                             rgbBlendOp(BlendOp::Add),
                             srcRgb(BlendFactor::One),
                             dstRgb(BlendFactor::Zero),
                             alphaBlendOp(BlendOp::Add),
                             srcAlpha(BlendFactor::One),
                             dstAlpha(BlendFactor::Zero),
                             writeMask(WriteMask::All) { };

        bool blendEnabled : 1;
        BlendOp rgbBlendOp : 3;
        BlendFactor srcRgb : 4;
        BlendFactor dstRgb : 4;
        BlendOp alphaBlendOp : 3;
        BlendFactor srcAlpha : 4;
        BlendFactor dstAlpha : 4;
        WriteMask writeMask : 4;
    }; // 4 bytes

    struct RasterizerDesc
    {
        RasterizerDesc() : fillMode(FillMode::Solid),
                             cullMode(CullMode::Back),
                             isFrontCcw(false),
                             depthClipEnabled(true),
                             multisampleEnabled(false),
                             linesAAEnabled(false),
                             conservativeRasterEnabled(false),
                             slopeScaledDepthBias(0.0f),
                             depthBias(0.0f),
                             forcedSampleCount(0) { };

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
        BlendDesc() : independentBlendEnabled(false),
                      alphaToCoverageEnabled(false) { };

        bool independentBlendEnabled : 1;
        bool alphaToCoverageEnabled : 1;
        std::array<RTBlendStateDesc, MAX_RENDER_TARGETS_COUNT> rtBlend;
    };

    struct DepthStencilDesc
    {
        enum class DepthAccess : uint8_t
        {
            None = 0,
            Read = 1,
            Write = 2,
            ReadWrite = Read | Write,
        }; // 2 bits
        ENUM_CLASS_FRIEND_BITWISE_OPS(DepthAccess);

        enum class ComparisonFunc : uint8_t
        {
            Never, // Comparison always fails
            Always, // Comparison always succeeds
            Less, // Passes if source is less than the destination
            Equal, // Passes if source is equal to the destination
            NotEqual, // Passes if source is not equal to the destination
            LessEqual, // Passes if source is less than or equal to the destination
            Greater, // Passes if source is greater than to the destination
            GreaterEqual, // Passes if source is greater than or equal to the destination
        }; // 3 bits

        enum class StencilOp : uint8_t
        {
            Keep, // Keep the stencil value
            Zero, // Set the stencil value to zero
            Replace, // Replace the stencil value with the reference value
            Increase, // Increase the stencil value by one, wrap if necessary
            IncreaseSaturate, /// Increase the stencil value by one, clamp if necessary
            Decrease, // Decrease the stencil value by one, wrap if necessary
            DecreaseSaturate, // Decrease the stencil value by one, clamp if necessary
            Invert //  Invert the stencil data (bitwise not)
        }; // 3 bits

        struct StencilDesc
        {
            StencilDesc() : func(ComparisonFunc::Always),
                            stencilFailOp(StencilOp::Keep),
                            depthFailOp(StencilOp::Keep),
                            depthStencilPassOp(StencilOp::Keep) { };

            ComparisonFunc func : 3; // Stencil comparison function
            StencilOp stencilFailOp : 3; // Stencil operation in case stencil test fails
            StencilOp depthFailOp : 3; /// Stencil operation in case stencil test passes but depth test fails
            StencilOp depthStencilPassOp : 3; // Stencil operation in case stencil and depth tests pass
        }; // 2 bytes

        DepthAccess depthAccess : 2;
        ComparisonFunc depthFunc : 3;
        bool stencilEnabled : 1;
        StencilDesc stencilFront;
        StencilDesc stencilBack;
        uint8_t stencilRef = 0;
        uint8_t stencilReadMask = 0xFF;
        uint8_t stencilWriteMask = 0xFF;
    };

    enum class PrimitiveTopology : uint8_t
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,

        LineListAdj,
        LineStripAdj,
        TriangleListAdj,
        TriangleStripAdj,

        PathListControlPoint_1,
        PathListControlPoint_2,
        PathListControlPoint_3,
        PathListControlPoint_4,
        PathListControlPoint_5,
        PathListControlPoint_6,
        PathListControlPoint_7,
        PathListControlPoint_8,
        PathListControlPoint_9,
        PathListControlPoint_10,
        PathListControlPoint_11,
        PathListControlPoint_12,
        PathListControlPoint_13,
        PathListControlPoint_14,
        PathListControlPoint_15,
        PathListControlPoint_16,
        PathListControlPoint_17,
        PathListControlPoint_18,
        PathListControlPoint_19,
        PathListControlPoint_20,
        PathListControlPoint_21,
        PathListControlPoint_22,
        PathListControlPoint_23,
        PathListControlPoint_24,
        PathListControlPoint_25,
        PathListControlPoint_26,
        PathListControlPoint_27,
        PathListControlPoint_28,
        PathListControlPoint_29,
        PathListControlPoint_30,
        PathListControlPoint_31,
        PathListControlPoint_32
    };

    struct GraphicPipelineStateDesc
    {
        BlendDesc blendDesc;
        RasterizerDesc rasterizerDesc;
        DepthStencilDesc depthStencilDesc;
        FramebufferDesc framebufferDesc;
        PrimitiveTopology primitiveTopology;
    };

}