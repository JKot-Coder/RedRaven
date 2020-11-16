#pragma once

namespace OpenDemo
{
    namespace Render
    {
        class Object;

        class Fence;
        class CommandContext;

        class SwapChain;
        struct SwapChainDescription;

        class Resource;
        enum class ResourceFormat : uint32_t;

        class ResourceView;
        struct ResourceViewDescription;
        class RenderTargetView;

        class Texture;
        struct TextureDescription;
        enum class TextureDimension : uint32_t;

        class Buffer;

        class Submission;

        struct Result;
    }
}