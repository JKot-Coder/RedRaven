#include "DeviceContext.hpp"

#include "gapi/Device.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "gapi_diligent/Device.hpp"

namespace RR::RenderLoom
{
    DeviceContext::~DeviceContext() { }

    void DeviceContext::Init(const GAPI::DeviceDescription& description)
    {
        ASSERT(!inited);
        // TODO: device should be belong to submission thread
        device = GAPI::Device::Create(description, "Primary");

        // Todo: select device based on description
        // Todo: do it in submission thread
        if(!GAPI::Diligent::InitDevice(device))
        {
            Log::Format::Error("Failed to initialize device");
            return;
        }

        inited = true;
    }

    void DeviceContext::Present(const GAPI::SwapChain::SharedPtr& swapChain)
    {
        device->Present(swapChain);
    }

    void DeviceContext::MoveToNextFrame(uint64_t frameIndex)
    {
        device->MoveToNextFrame(frameIndex);
    }

    GAPI::Texture::SharedPtr DeviceContext::CreateTexture(
        const GAPI::GpuResourceDescription& desc,
        const Common::IDataBuffer::SharedPtr& initialData,
        const std::string& name)
    {
        ASSERT(inited);

        auto resource = GAPI::Texture::Create(shared_from_this(), desc, initialData, name);
        device->InitTexture(resource);

        return resource;
    }

    GAPI::RenderTargetView::SharedPtr DeviceContext::CreateRenderTargetView(
        const GAPI::Texture::SharedPtr& texture,
        const GAPI::GpuResourceViewDescription& desc) const
    {
        ASSERT(inited);

        auto resource = GAPI::RenderTargetView::Create(texture, desc);
        device->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::DepthStencilView::SharedPtr DeviceContext::CreateDepthStencilView(
        const eastl::shared_ptr<GAPI::Texture>& texture,
        const GAPI::GpuResourceViewDescription& desc) const
    {
        ASSERT(inited);

        auto resource = GAPI::DepthStencilView::Create(texture, desc);
        device->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::ShaderResourceView::SharedPtr DeviceContext::CreateShaderResourceView(
        const GAPI::GpuResource::SharedPtr& gpuResource,
        const GAPI::GpuResourceViewDescription& desc) const
    {
        ASSERT(inited);

        auto resource = GAPI::ShaderResourceView::Create(gpuResource, desc);
        device->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::UnorderedAccessView::SharedPtr DeviceContext::CreateUnorderedAccessView(
        const GAPI::GpuResource::SharedPtr& gpuResource,
        const GAPI::GpuResourceViewDescription& desc) const
    {
        ASSERT(inited);

        auto resource = GAPI::UnorderedAccessView::Create(gpuResource, desc);
        device->InitGpuResourceView(*resource.get());

        return resource;;
    }

    GAPI::SwapChain::SharedPtr DeviceContext::CreateSwapchain(const GAPI::SwapChainDescription& description) const
    {
        ASSERT(inited);

        auto resource = GAPI::SwapChain::Create(description);
        device->InitSwapChain(*resource.get());

        return resource;
    }
}
