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

        template <typename T, bool IsNamed = true>
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

        class MemoryAllocation;
        enum class MemoryAllocationType : uint32_t;
        class IntermediateMemory;

        class Texture;
        struct TextureDescription;
        enum class TextureDimension : uint32_t;

        class Buffer;
        struct BufferDescription;
    }

    namespace Render
    {
        class RenderContext;
    }
}