#pragma once

namespace RR
{
    namespace GAPI
    {
        class Object;

        class Fence;
        class Texture;
        class Buffer;

        class Framebuffer;
        struct FramebufferDescription;

        class SwapChain;
        struct SwapChainDescription;

        class CommandQueue;
        enum class CommandQueueType : uint32_t;

        class CommandList;
        class CopyCommandList;
        class ComputeCommandList;
        class GraphicsCommandList;

        struct PresentOptions;

        template <typename T, bool IsNamed>
        class Resource;

        class GpuResource;
        struct GpuResourceDescription;
        enum class GpuResourceDimension : uint32_t;
        enum class GpuResourceBindFlags : uint32_t;
        enum class GpuResourceFormat : uint32_t;
        enum class GpuResourceUsage : uint32_t;
        enum class MultisampleType : uint32_t;

        class GpuResourceView;
        struct GpuResourceViewDescription;

        class ShaderResourceView;
        class RenderTargetView;
        class DepthStencilView;
        class UnorderedAccessView;

        class MemoryAllocation;
        enum class MemoryAllocationType : uint32_t;
        class CpuResourceData;
    }

    namespace Render
    {
        class DeviceContext;
    }
}