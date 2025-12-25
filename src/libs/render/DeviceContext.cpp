#include "DeviceContext.hpp"

#include "gapi/Device.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Shader.hpp"
#include "gapi/Buffer.hpp"
#include "gapi/Texture.hpp"
#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/PipelineState.hpp"

#include "render/CommandEncoder.hpp"
#include "render/Effect.hpp"

#include "gapi_webgpu/Device.hpp"
#include "gapi_dx12/Device.hpp"

namespace RR::Render
{
    DeviceContext::DeviceContext() {};
    DeviceContext::~DeviceContext() { Terminate();  }

    bool DeviceContext::Init(const GAPI::DeviceDesc& desc)
    {
        ASSERT(!inited);

        auto device = GAPI::Device::Create(desc, "Primary");
        submission.Start(eastl::move(device), SubmissionThreadMode::Enabled);

        inited = false;
        submission.ExecuteAwait([this](GAPI::Device& device) {
            if(!GAPI::WebGPU::InitDevice(device))
            {
                Log::Format::Error("Failed to initialize device");
                return;
            }

            multiThreadDevice = static_cast<GAPI::Device::IMultiThreadDevice*>(&device);
            inited = true;
        });

        if(!inited)
            submission.Terminate();

        return inited;
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

    void DeviceContext::Compile(Render::CommandEncoder& commandContext)
    {
        ASSERT(inited);

        multiThreadDevice->Compile(commandContext.GetCommandList());
    }

    Render::CommandEncoder::UniquePtr DeviceContext::CreateCommandEncoder(const std::string& name) const
    {
        ASSERT(inited);

        GAPI::CommandList commandList(name);
        multiThreadDevice->InitCommandList(commandList);
        auto resource = Render::CommandEncoder::Create(eastl::move(commandList));

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

    GAPI::Buffer::SharedPtr DeviceContext::CreateBuffer(const GAPI::GpuResourceDesc& desc, const GAPI::BufferData* initialData, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = GAPI::Buffer::Create(desc, name);
        multiThreadDevice->InitBuffer(*resource.get(), initialData);

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

    GAPI::Texture::SharedPtr DeviceContext::CreateSwapChainBackBuffer(const GAPI::SwapChain* swapchain, const GAPI::GpuResourceDesc& desc, const std::string& name) const
    {
        ASSERT(inited);

        ASSERT(swapchain);
        ASSERT(desc.dimension == GAPI::GpuResourceDimension::Texture2D);
        ASSERT(desc.usage == GAPI::GpuResourceUsage::Default);
        ASSERT(desc.GetNumSubresources() == 1);
        ASSERT(desc.bindFlags == GAPI::GpuResourceBindFlags::RenderTarget);

        return GAPI::Texture::Create(desc, nullptr, name);
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
