#pragma once

namespace RR
{
    namespace GAPI
    {
        class Device;
        class DeviceDescription;

        class Fence;
        class Texture;
        class Buffer;

        class Framebuffer;
        struct FramebufferDesc;

        class SwapChain;
        struct SwapChainDescription;

        class CommandQueue;
        enum class CommandQueueType : uint32_t;

        class CommandContext;
        class GraphicsCommandContext;
        class ComputeCommandContext;
        class CopyCommandContext;

        class CommandList;
        class CopyCommandList;
        class ComputeCommandList;
        class GraphicsCommandList;

        struct PresentOptions;

        template <typename T, bool IsNamed>
        class Resource;

        class GpuResource;
        struct GpuResourceDescription;
        struct GpuResourceFootprint;

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
    }

    namespace Render
    {
        class DeviceContext;
    }

    namespace RenderLoom
    {
        class DeviceContext;
    }
}