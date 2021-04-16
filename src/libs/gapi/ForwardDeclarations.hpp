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

        template <typename T, bool IsNamed>
        class Resource;

        class GpuResource;
        struct GpuResourceDescription;
        enum class GpuResourceDimension : uint32_t;
        enum class GpuResourceBindFlags : uint32_t;
        enum class GpuResourceFormat : uint32_t;
        enum class GpuResourceCpuAccess : uint32_t;

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
        class CpuResourceData;

        class Texture;
        class Buffer;
    }

    namespace Render
    {
        class DeviceContext;
    }
}