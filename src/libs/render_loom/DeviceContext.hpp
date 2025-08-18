#pragma once

#include "gapi/ForwardDeclarations.hpp"

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

namespace RR::RenderLoom
{
    class DeviceContext : public eastl::enable_shared_from_this<DeviceContext>
    {
    public:
        using SharedPtr = eastl::shared_ptr<DeviceContext>;
        using SharedConstPtr = eastl::shared_ptr<const DeviceContext>;

        ~DeviceContext();

        void Init(const GAPI::DeviceDescription& description);

        void Present(GAPI::SwapChain* swapChain);
        void MoveToNextFrame(uint64_t frameIndex);

        eastl::unique_ptr<GAPI::GraphicsCommandContext> CreateGraphicsCommandContext(const std::string& name) const;
        eastl::unique_ptr<GAPI::Texture> CreateTexture(const GAPI::GpuResourceDescription& desc, const eastl::shared_ptr<Common::IDataBuffer>& initialData, const std::string& name);
        eastl::unique_ptr<GAPI::RenderTargetView> CreateRenderTargetView(const GAPI::Texture* texture, const GAPI::GpuResourceViewDescription& desc) const;
        eastl::unique_ptr<GAPI::DepthStencilView> CreateDepthStencilView(const GAPI::Texture* texture, const GAPI::GpuResourceViewDescription& desc) const;
        eastl::unique_ptr<GAPI::ShaderResourceView> CreateShaderResourceView(const GAPI::GpuResource* gpuResource, const GAPI::GpuResourceViewDescription& desc) const;
        eastl::unique_ptr<GAPI::UnorderedAccessView> CreateUnorderedAccessView(const GAPI::GpuResource* gpuResource, const GAPI::GpuResourceViewDescription& desc) const;
        eastl::unique_ptr<GAPI::SwapChain> CreateSwapchain(const GAPI::SwapChainDescription& description) const;

    public:
        static SharedPtr Create()
        {
            return eastl::make_shared<DeviceContext>();
        }

    private:
        DeviceContext() = default;
        EASTL_FRIEND_MAKE_SHARED;

    private:
        bool inited = false;
        eastl::shared_ptr<GAPI::Device> device;
    };
}