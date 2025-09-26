#pragma once

namespace RR
{
    namespace GAPI
    {
        class Device;
        struct DeviceDesc;

        class Shader;
        struct ShaderDesc;
        enum class ShaderType : uint8_t;

        struct VertexLayout;

        class PipelineState;
        class IPipelineState;
        class GraphicPipelineState;
        struct GraphicPipelineStateDesc;

        struct RenderPassDesc;

        struct BufferData;

        class Fence;
        class Texture;
        class Buffer;

        class SwapChain;
        struct SwapChainDesc;

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
        struct GpuResourceDesc;
        struct GpuResourceFootprint;

        enum class GpuResourceDimension : uint32_t;
        enum class GpuResourceBindFlags : uint32_t;
        enum class GpuResourceFormat : uint32_t;
        enum class GpuResourceUsage : uint32_t;
        enum class MultisampleType : uint32_t;

        class GpuResourceView;
        struct GpuResourceViewDesc;

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