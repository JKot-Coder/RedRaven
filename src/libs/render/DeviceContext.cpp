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
#include "render/SwapChain.hpp"

#include "gapi_webgpu/Device.hpp"
#include "gapi_dx12/Device.hpp"

namespace RR::Render
{
    DeviceContext::DeviceContext() {};
    DeviceContext::~DeviceContext() { Terminate();  }

    bool DeviceContext::Init(const GAPI::DeviceDesc& desc)
    {
        ASSERT(!inited);

        auto device = eastl::unique_ptr<GAPI::Device>(new GAPI::Device(desc, "Primary"));
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

    void DeviceContext::Present(Render::SwapChain& swapChain)
    {
        submission.ExecuteAwait([&swapChain](GAPI::Device& device) {
            device.Present(swapChain.GetSwapChain());
            swapChain.UpdateBackBuffer();
        });
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

        auto resource = GAPI::CommandQueue::UniquePtr(new GAPI::CommandQueue(type, name));
        multiThreadDevice->InitCommandQueue(*resource.get());

        return resource;
    }

    void DeviceContext::Compile(Render::CommandEncoder& commandContext)
    {
        ASSERT(inited);

        multiThreadDevice->Compile(commandContext.GetCommandList());
        commandContext.Reset();
    }

    eastl::unique_ptr<Render::CommandEncoder> DeviceContext::CreateCommandEncoder(const std::string& name) const
    {
        ASSERT(inited);

        GAPI::CommandList commandList(name);
        multiThreadDevice->InitCommandList(commandList);
        auto resource = eastl::unique_ptr<Render::CommandEncoder>(new Render::CommandEncoder(eastl::move(commandList)));

        return resource;
    }

    eastl::unique_ptr<Render::Effect> DeviceContext::CreateEffect(const std::string& name, EffectDesc&& effectDesc) const
    {
        ASSERT(inited);
        // Why it's here?
        return eastl::unique_ptr<Render::Effect>(new Render::Effect(name, eastl::move(effectDesc)));
    }

    GAPI::Shader::UniquePtr DeviceContext::CreateShader(const GAPI::ShaderDesc& desc, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = GAPI::Shader::UniquePtr(new GAPI::Shader(desc, name));
        multiThreadDevice->InitShader(*resource.get());

        return resource;
    }

    GAPI::Buffer::UniquePtr DeviceContext::CreateBuffer(const GAPI::GpuResourceDesc& desc, const GAPI::BufferData* initialData, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = GAPI::Buffer::UniquePtr(new GAPI::Buffer(desc, name));
        multiThreadDevice->InitBuffer(*resource.get(), initialData);

        return resource;
    }

    GAPI::Texture::UniquePtr DeviceContext::CreateTexture(
        const GAPI::GpuResourceDesc& desc,
        const Common::IDataBuffer::SharedPtr& initialData,
        const std::string& name)
    {
        ASSERT(inited);

        auto resource = GAPI::Texture::UniquePtr(new GAPI::Texture(desc, initialData, name));
        multiThreadDevice->InitTexture(*static_cast<GAPI::Texture*>(resource.get()));

        return resource;
    }

    GAPI::RenderTargetView::UniquePtr DeviceContext::CreateRenderTargetView(
        GAPI::Texture& texture,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);

        auto resource = GAPI::RenderTargetView::UniquePtr(new GAPI::RenderTargetView(texture, desc));
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::DepthStencilView::UniquePtr DeviceContext::CreateDepthStencilView(
        GAPI::Texture& texture,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);

        auto resource = GAPI::DepthStencilView::UniquePtr(new GAPI::DepthStencilView(texture, desc));
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::ShaderResourceView::UniquePtr DeviceContext::CreateShaderResourceView(
        GAPI::GpuResource& gpuResource,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);

        auto resource = GAPI::ShaderResourceView::UniquePtr(new GAPI::ShaderResourceView(gpuResource, desc));
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::UnorderedAccessView::UniquePtr DeviceContext::CreateUnorderedAccessView(
        GAPI::GpuResource& gpuResource,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);

        auto resource = GAPI::UnorderedAccessView::UniquePtr(new GAPI::UnorderedAccessView(gpuResource, desc));
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    GAPI::SwapChain::UniquePtr DeviceContext::CreateSwapchain(const GAPI::SwapChainDesc& desc) const
    {
        ASSERT(inited);

        auto resource = GAPI::SwapChain::UniquePtr(new GAPI::SwapChain(desc));
        multiThreadDevice->InitSwapChain(*resource.get());

        return resource;
    }

    GAPI::Texture::UniquePtr DeviceContext::CreateSwapChainBackBuffer(GAPI::SwapChain& swapchain, const GAPI::GpuResourceDesc& desc, const std::string& name) const
    {
        ASSERT(inited);

        ASSERT(desc.dimension == GAPI::GpuResourceDimension::Texture2D);
        ASSERT(desc.usage == GAPI::GpuResourceUsage::Default);
        ASSERT(desc.GetNumSubresources() == 1);
        ASSERT(desc.bindFlags == GAPI::GpuResourceBindFlags::RenderTarget);

        auto resource = GAPI::Texture::UniquePtr(new GAPI::Texture(desc, nullptr, name));
        multiThreadDevice->InitSwapChainBackBuffer(swapchain, *resource.get());

        return resource;
    }

    GAPI::GraphicPipelineState::UniquePtr DeviceContext::CreatePipelineState(const GAPI::GraphicPipelineStateDesc& desc, const std::string& name) const
    {
        ASSERT(inited);

        ASSERT_MSG(desc.vs, "VS is not set in graphic pipeline state: \"{}\"", name);
        ASSERT_MSG(desc.ps, "PS is not set in graphic pipeline state: \"{}\"", name);

        auto resource = GAPI::GraphicPipelineState::UniquePtr(new GAPI::GraphicPipelineState(desc, name));
        multiThreadDevice->InitPipelineState(*resource.get());

        return resource;
    }
}
