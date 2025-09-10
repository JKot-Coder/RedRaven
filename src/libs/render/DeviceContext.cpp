#include "DeviceContext.hpp"

#include "gapi/Device.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Shader.hpp"
#include "gapi/Texture.hpp"
#include "gapi/CommandList2.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/PipelineState.hpp"

#include "render/CommandContext.hpp"
#include "render/Effect.hpp"

#include "gapi_diligent/Device.hpp"

namespace RR::Render
{
    DeviceContext::DeviceContext() {};
    DeviceContext::~DeviceContext() { Terminate();  }

    void DeviceContext::Init(const GAPI::DeviceDesc& desc)
    {
        ASSERT(!inited);

        auto device = GAPI::Device::Create(desc, "Primary");
        submission.Start(eastl::move(device), SubmissionThreadMode::Enabled);

        bool result = false;
        submission.ExecuteAwait([&result, this](GAPI::Device& device) {
            if(!GAPI::Diligent::InitDevice(device))
            {
                Log::Format::Error("Failed to initialize device");
                return;
            }

            this->multiThreadDevice = static_cast<GAPI::Device::IMultiThreadDevice*>(&device);
            result = true;
        });

        inited = result;
    }

    void DeviceContext::Terminate()
    {
        if(!inited)
            return;

        submission.Terminate();
        inited = false;
    }

    void DeviceContext::Present(GAPI::SwapChain* swapChain)
    {
        ASSERT(swapChain);
        submission.ExecuteAwait([swapChain](GAPI::Device& device) { device.Present(swapChain); });
    }

    void DeviceContext::MoveToNextFrame(uint64_t frameIndex)
    {
        submission.ExecuteAwait([frameIndex](GAPI::Device& device) { device.MoveToNextFrame(frameIndex); });
    }

    void DeviceContext::ResizeSwapChain(GAPI::SwapChain* swapchain, uint32_t width, uint32_t height)
    {
        swapchain->Resize(width, height);
    }

    GAPI::CommandQueue::UniquePtr DeviceContext::CreateCommandQueue(GAPI::CommandQueueType type, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = GAPI::CommandQueue::Create(type, name);
        multiThreadDevice->InitCommandQueue(*resource.get());

        return resource;
    }

    void DeviceContext::Compile(GAPI::CommandList2& commandList)
    {
        ASSERT(inited);

        multiThreadDevice->Compile(commandList);
    }

    void DeviceContext::Submit(GAPI::CommandQueue* commandQueue, GAPI::CommandList2& commandList)
    {
        ASSERT(inited);
        ASSERT(commandQueue);

        submission.Submit(commandQueue, commandList);
    }

    Render::GraphicsCommandContext::UniquePtr DeviceContext::CreateGraphicsCommandContext(const std::string& name) const
    {
        ASSERT(inited);

        GAPI::CommandList2 commandList(name);
        multiThreadDevice->InitCommandList2(commandList);
        auto resource = Render::GraphicsCommandContext::Create(eastl::move(commandList));

        return resource;
    }

    Render::Effect::UniquePtr DeviceContext::CreateEffect(const std::string& name, EffectDesc&& effectDesc) const
    {
        ASSERT(inited);
        // Why it's here?
        return Render::Effect::Create(name, eastl::move(effectDesc));
    }

    GAPI::Shader::UniquePtr DeviceContext::CreateShader(const GAPI::ShaderDesc& desc, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = GAPI::Shader::Create(desc, name);
        multiThreadDevice->InitShader(*resource.get());

        return resource;
    }

    GAPI::Texture::SharedPtr DeviceContext::CreateTexture(
        const GAPI::GpuResourceDesc& desc,
        const Common::IDataBuffer::SharedPtr& initialData,
        const std::string& name)
    {
        ASSERT(inited);

        auto resource = GAPI::Texture::Create(desc, initialData, name);
        multiThreadDevice->InitTexture(*static_cast<GAPI::Texture*>(resource.get()));

        return resource;
    }

    GAPI::RenderTargetView::UniquePtr DeviceContext::CreateRenderTargetView(
        const eastl::shared_ptr<GAPI::Texture>& texture,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);
        ASSERT(texture);

        auto resource = GAPI::RenderTargetView::Create(texture, desc);
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::DepthStencilView::UniquePtr DeviceContext::CreateDepthStencilView(
        const eastl::shared_ptr<GAPI::Texture>& texture,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);
        ASSERT(texture);

        auto resource = GAPI::DepthStencilView::Create(texture, desc);
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::ShaderResourceView::UniquePtr DeviceContext::CreateShaderResourceView(
        const eastl::shared_ptr<GAPI::GpuResource>& gpuResource,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);
        ASSERT(gpuResource);

        auto resource = GAPI::ShaderResourceView::Create(gpuResource, desc);
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::UnorderedAccessView::UniquePtr DeviceContext::CreateUnorderedAccessView(
        const eastl::shared_ptr<GAPI::GpuResource>& gpuResource,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);
        ASSERT(gpuResource);

        auto resource = GAPI::UnorderedAccessView::Create(gpuResource, desc);
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::SwapChain::UniquePtr DeviceContext::CreateSwapchain(const GAPI::SwapChainDesc& desc) const
    {
        ASSERT(inited);

        auto resource = GAPI::SwapChain::Create(desc);
        multiThreadDevice->InitSwapChain(*resource.get());

        return resource;
    }

    GAPI::Texture::SharedPtr DeviceContext::CreateSwapChainBackBuffer(const GAPI::SwapChain* swapchain, uint32_t backBufferIndex, const GAPI::GpuResourceDesc& desc, const std::string& name) const
    {
        ASSERT(inited);

        ASSERT(swapchain);
        ASSERT(desc.dimension == GAPI::GpuResourceDimension::Texture2D);
        ASSERT(desc.usage == GAPI::GpuResourceUsage::Default);
        ASSERT(desc.GetNumSubresources() == 1);
        ASSERT(desc.bindFlags == GAPI::GpuResourceBindFlags::RenderTarget);

        auto resource = GAPI::Texture::Create(desc, nullptr, name);
        swapchain->InitBackBufferTexture(backBufferIndex, *resource.get());

        return resource;
    }

    eastl::shared_ptr<GAPI::Texture> DeviceContext::CreateSwapChainDepthBuffer(const GAPI::SwapChain* swapchain, const GAPI::GpuResourceDesc& desc) const
    {
        ASSERT(inited);

        ASSERT(swapchain);
        ASSERT(desc.dimension == GAPI::GpuResourceDimension::Texture2D);
        ASSERT(desc.usage == GAPI::GpuResourceUsage::Default);
        ASSERT(desc.GetNumSubresources() == 1);
        ASSERT(desc.bindFlags == GAPI::GpuResourceBindFlags::DepthStencil);

        auto resource = GAPI::Texture::Create(desc, nullptr, "SwapChain Depth Buffer");
        swapchain->InitDepthBufferTexture(*resource.get());

        return resource;
    }

    GAPI::GraphicPipelineState::UniquePtr DeviceContext::CreatePipelineState(const GAPI::GraphicPipelineStateDesc& desc, const std::string& name) const
    {
        ASSERT(inited);

        ASSERT_MSG(desc.vs, "VS is not set in graphic pipeline state: \"{}\"", name);
        ASSERT_MSG(desc.ps, "PS is not set in graphic pipeline state: \"{}\"", name);

        auto resource = GAPI::GraphicPipelineState::Create(desc, name);
        multiThreadDevice->InitPipelineState(*resource.get());

        return resource;
    }
}
