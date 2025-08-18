#include "DeviceContext.hpp"

#include "gapi/Device.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"
#include "gapi/CommandList2.hpp"

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

    void DeviceContext::Present(GAPI::SwapChain* swapChain)
    {
        device->Present(swapChain);
    }

    void DeviceContext::MoveToNextFrame(uint64_t frameIndex)
    {
        device->MoveToNextFrame(frameIndex);
    }

    GAPI::GraphicsCommandContext::UniquePtr DeviceContext::CreateGraphicsCommandContext(const std::string& name) const
    {
        ASSERT(inited);

        auto resource = GAPI::GraphicsCommandContext::Create(name);
        device->InitCommandContext(*resource.get());

        return resource;
    }

    GAPI::Texture::UniquePtr DeviceContext::CreateTexture(
        const GAPI::GpuResourceDescription& desc,
        const Common::IDataBuffer::SharedPtr& initialData,
        const std::string& name)
    {
        ASSERT(inited);

        auto resource = GAPI::Texture::Create(shared_from_this(), desc, initialData, name);
        device->InitTexture(*static_cast<GAPI::Texture*>(resource.get()));

        return resource;
    }

    GAPI::RenderTargetView::UniquePtr DeviceContext::CreateRenderTargetView(
        const GAPI::Texture* texture,
        const GAPI::GpuResourceViewDescription& desc) const
    {
        ASSERT(inited);
        ASSERT(texture);

        auto resource = GAPI::RenderTargetView::Create(*texture, desc);
        device->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::DepthStencilView::UniquePtr DeviceContext::CreateDepthStencilView(
        const GAPI::Texture* texture,
        const GAPI::GpuResourceViewDescription& desc) const
    {
        ASSERT(inited);
        ASSERT(texture);

        auto resource = GAPI::DepthStencilView::Create(*texture, desc);
        device->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::ShaderResourceView::UniquePtr DeviceContext::CreateShaderResourceView(
        const GAPI::GpuResource* gpuResource,
        const GAPI::GpuResourceViewDescription& desc) const
    {
        ASSERT(inited);
        ASSERT(gpuResource);

        auto resource = GAPI::ShaderResourceView::Create(*gpuResource, desc);
        device->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::UnorderedAccessView::UniquePtr DeviceContext::CreateUnorderedAccessView(
        const GAPI::GpuResource* gpuResource,
        const GAPI::GpuResourceViewDescription& desc) const
    {
        ASSERT(inited);
        ASSERT(gpuResource);

        auto resource = GAPI::UnorderedAccessView::Create(*gpuResource, desc);
        device->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::SwapChain::UniquePtr DeviceContext::CreateSwapchain(const GAPI::SwapChainDescription& description) const
    {
        ASSERT(inited);

        auto resource = GAPI::SwapChain::Create(description);
        device->InitSwapChain(*resource.get());

        return resource;
    }
}
