#include "DeviceContext.hpp"

#include "gapi/Device.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Shader.hpp"
#include "gapi/Texture.hpp"
#include "gapi/CommandList2.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/PipelineState.hpp"

#include "gapi_diligent/Device.hpp"

namespace RR::Render
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

    void DeviceContext::ResizeSwapChain(GAPI::SwapChain* swapchain, uint32_t width, uint32_t height)
    {
        swapchain->Resize(width, height);
    }

    GAPI::CommandQueue::UniquePtr DeviceContext::CreateCommandQueue(GAPI::CommandQueueType type, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = GAPI::CommandQueue::Create(type, name);
        device->InitCommandQueue(*resource.get());

        return resource;
    }

    void DeviceContext::Compile(GAPI::GraphicsCommandContext* commandContext)
    {
        ASSERT(inited);

        device->Compile(commandContext);
    }

    GAPI::GraphicsCommandContext::UniquePtr DeviceContext::CreateGraphicsCommandContext(const std::string& name) const
    {
        ASSERT(inited);

        auto resource = GAPI::GraphicsCommandContext::Create(name);
        device->InitCommandContext(*resource.get());

        return resource;
    }

    GAPI::Shader::UniquePtr DeviceContext::CreateShader(const GAPI::ShaderDescription& description, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = GAPI::Shader::Create(description, name);
        device->InitShader(*resource.get());

        return resource;
    }

    GAPI::Texture::UniquePtr DeviceContext::CreateTexture(
        const GAPI::GpuResourceDescription& desc,
        const Common::IDataBuffer::SharedPtr& initialData,
        const std::string& name)
    {
        ASSERT(inited);

        auto resource = GAPI::Texture::Create(desc, initialData, name);
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

    GAPI::Texture::UniquePtr DeviceContext::CreateSwapChainBackBuffer(const GAPI::SwapChain* swapchain, uint32_t backBufferIndex, const GAPI::GpuResourceDescription& desc, const std::string& name) const
    {
        ASSERT(inited);

        ASSERT(swapchain);
        ASSERT(desc.dimension == GAPI::GpuResourceDimension::Texture2D);
        ASSERT(desc.usage == GAPI::GpuResourceUsage::Default);
        ASSERT(desc.GetNumSubresources() == 1);

        auto resource = GAPI::Texture::Create(desc, nullptr, name);
        swapchain->InitBackBufferTexture(backBufferIndex, *resource.get());

        return resource;
    }

    GAPI::GraphicPipelineState::UniquePtr DeviceContext::CreatePipelineState(const GAPI::GraphicPipelineStateDesc& description, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = GAPI::GraphicPipelineState::Create(description, name);
        device->InitPipelineState(*resource.get());

        return resource;
    }
}
