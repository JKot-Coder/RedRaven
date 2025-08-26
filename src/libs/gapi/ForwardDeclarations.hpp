#pragma once

namespace RR
{
    namespace GAPI
    {
        class Device;
        struct DeviceDescription;

        class Shader;
        struct ShaderDescription;

        class PipelineState;
        class IPipelineState;
        class GraphicPipelineState;
        struct GraphicPipelineStateDesc;

        struct RenderPassDesc;

        class Fence;
        class Texture;
        class Buffer;

        class SwapChain;
        struct SwapChainDescription;

        class CommandQueue;
        enum class CommandQueueType : uint32_t;

        class CommandList2;

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

    namespace Render
    {
        class DeviceContext;
    }
}