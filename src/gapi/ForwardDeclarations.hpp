#pragma once

namespace OpenDemo
{
    namespace GAPI
    {
        class Buffer;
        class CommandList;
        class CopyCommandList;
        class ComputeCommandList;
        class GraphicsCommandList;

        struct PresentOptions;

        class CommandQueue;
        enum class CommandQueueType : uint32_t;
        class Fence;
        class Object;

        class Resource;
        enum class ResourceBindFlags : uint32_t;
        enum class ResourceFormat : uint32_t;

        class ResourceView;
        struct ResourceViewDescription;

        class RenderTargetView;

        class SwapChain;
        struct SwapChainDescription;

        class Texture;
        struct TextureDescription;
        enum class TextureDimension : uint32_t;

        struct Result;
    }

    namespace Render
    {
        class RenderContext;
    }
}