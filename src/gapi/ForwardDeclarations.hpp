#pragma once

namespace OpenDemo
{
    namespace Render
    {
        class Buffer;
        class CommandList;
        class CommandQueue;
        class Fence;
        class Object;

        class Resource;
        enum class ResourceFormat : uint32_t;

        class ResourceView;
        struct ResourceViewDescription;

        class RenderTargetView;

        class Submission;

        class SwapChain;
        struct SwapChainDescription;

        class Texture;
        struct TextureDescription;
        enum class TextureDimension : uint32_t;

        struct Result;     
    }
}