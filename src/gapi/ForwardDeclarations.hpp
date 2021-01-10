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

        template <typename T>
        class Resource;

        class GpuResource;
        enum class GpuResourceBindFlags : uint32_t;
        enum class GpuResourceFormat : uint32_t;

        class GpuResourceView;
        struct GpuResourceViewDescription;

        class ShaderResourceView;
        class RenderTargetView;
        class DepthStencilView;
        class UnorderedAccessView;

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