#pragma once

#include "gapi/ForwardDeclarations.hpp"

#include "common/Singleton.hpp"

namespace RR
{
    namespace Common
    {
        class IDataBuffer;
    }
    namespace GAPI
    {
        struct GpuResourceViewDescription;
    }
}

namespace RR::Render
{
    class DeviceContext : public Common::Singleton<DeviceContext>
    {
    public:
        using SharedPtr = eastl::shared_ptr<DeviceContext>;
        using SharedConstPtr = eastl::shared_ptr<const DeviceContext>;

        DeviceContext() = default;
        ~DeviceContext();

        void Init(const GAPI::DeviceDescription& description);

        void Present(GAPI::SwapChain* swapChain);
        void MoveToNextFrame(uint64_t frameIndex);
        void Compile(GAPI::GraphicsCommandContext* commandContext);

        eastl::unique_ptr<GAPI::CommandQueue> CreateCommandQueue(GAPI::CommandQueueType type, const std::string& name) const;
        eastl::unique_ptr<GAPI::GraphicsCommandContext> CreateGraphicsCommandContext(const std::string& name) const;
        eastl::unique_ptr<GAPI::Texture> CreateTexture(const GAPI::GpuResourceDescription& desc, const eastl::shared_ptr<Common::IDataBuffer>& initialData, const std::string& name);
        eastl::unique_ptr<GAPI::RenderTargetView> CreateRenderTargetView(const GAPI::Texture* texture, const GAPI::GpuResourceViewDescription& desc) const;
        eastl::unique_ptr<GAPI::DepthStencilView> CreateDepthStencilView(const GAPI::Texture* texture, const GAPI::GpuResourceViewDescription& desc) const;
        eastl::unique_ptr<GAPI::ShaderResourceView> CreateShaderResourceView(const GAPI::GpuResource* gpuResource, const GAPI::GpuResourceViewDescription& desc) const;
        eastl::unique_ptr<GAPI::UnorderedAccessView> CreateUnorderedAccessView(const GAPI::GpuResource* gpuResource, const GAPI::GpuResourceViewDescription& desc) const;
        eastl::unique_ptr<GAPI::SwapChain> CreateSwapchain(const GAPI::SwapChainDescription& description) const;
        eastl::unique_ptr<GAPI::Texture> CreateSwapChainBackBuffer(const GAPI::SwapChain* swapchain, uint32_t backBufferIndex, const GAPI::GpuResourceDescription& desc, const std::string& name) const;

    private:
        bool inited = false;
        eastl::shared_ptr<GAPI::Device> device;
    };
}