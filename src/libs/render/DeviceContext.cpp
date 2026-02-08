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

    CommandQueueUniquePtr DeviceContext::CreateCommandQueue(GAPI::CommandQueueType type, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = CommandQueueUniquePtr(new GAPI::CommandQueue(type, name));
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

    ShaderUniquePtr DeviceContext::CreateShader(const GAPI::ShaderDesc& desc, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = ShaderUniquePtr(new GAPI::Shader(desc, name));
        multiThreadDevice->InitShader(*resource.get());

        return resource;
    }

    BufferUniquePtr DeviceContext::CreateBuffer(const GAPI::GpuResourceDesc& desc, const GAPI::BufferData* initialData, const std::string& name) const
    {
        ASSERT(inited);

        auto resource = BufferUniquePtr(new GAPI::Buffer(desc, name));
        multiThreadDevice->InitBuffer(*resource.get(), initialData);

        return resource;
    }

    TextureUniquePtr DeviceContext::CreateTexture(
        const GAPI::GpuResourceDesc& desc,
        const Common::IDataBuffer::SharedPtr& initialData,
        const std::string& name)
    {
        ASSERT(inited);

        auto resource = TextureUniquePtr(new GAPI::Texture(desc, initialData, name));
        multiThreadDevice->InitTexture(*static_cast<GAPI::Texture*>(resource.get()));

        return resource;
    }

    RenderTargetViewUniquePtr DeviceContext::CreateRenderTargetView(
        GAPI::Texture& texture,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);

        auto resource = RenderTargetViewUniquePtr(new GAPI::RenderTargetView(texture, desc));
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    DepthStencilViewUniquePtr DeviceContext::CreateDepthStencilView(
        GAPI::Texture& texture,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);

        auto resource = DepthStencilViewUniquePtr(new GAPI::DepthStencilView(texture, desc));
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    ShaderResourceViewUniquePtr DeviceContext::CreateShaderResourceView(
        GAPI::GpuResource& gpuResource,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);

        auto resource = ShaderResourceViewUniquePtr(new GAPI::ShaderResourceView(gpuResource, desc));
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    UnorderedAccessViewUniquePtr DeviceContext::CreateUnorderedAccessView(
        GAPI::GpuResource& gpuResource,
        const GAPI::GpuResourceViewDesc& desc) const
    {
        ASSERT(inited);

        auto resource = UnorderedAccessViewUniquePtr(new GAPI::UnorderedAccessView(gpuResource, desc));
        multiThreadDevice->InitGpuResourceView(*resource.get());

        return resource;
    }

    SwapChainUniquePtr DeviceContext::CreateSwapchain(const GAPI::SwapChainDesc& desc) const
    {
        ASSERT(inited);

        auto resource = SwapChainUniquePtr(new GAPI::SwapChain(desc));
        multiThreadDevice->InitSwapChain(*resource.get());

        return resource;
    }

    eastl::unique_ptr<GAPI::Texture> DeviceContext::CreateSwapChainBackBuffer(const GAPI::SwapChain* swapchain, const GAPI::GpuResourceDesc& desc, const std::string& name) const
    {
        ASSERT(inited);

        ASSERT(swapchain);
        ASSERT(desc.dimension == GAPI::GpuResourceDimension::Texture2D);
        ASSERT(desc.usage == GAPI::GpuResourceUsage::Default);
        ASSERT(desc.GetNumSubresources() == 1);
        ASSERT(desc.bindFlags == GAPI::GpuResourceBindFlags::RenderTarget);

        return eastl::unique_ptr<GAPI::Texture>(new GAPI::Texture(desc, nullptr, name));
    }

    GraphicPipelineStateUniquePtr DeviceContext::CreatePipelineState(const GAPI::GraphicPipelineStateDesc& desc, const std::string& name) const
    {
        ASSERT(inited);

        ASSERT_MSG(desc.vs, "VS is not set in graphic pipeline state: \"{}\"", name);
        ASSERT_MSG(desc.ps, "PS is not set in graphic pipeline state: \"{}\"", name);

        auto resource = GraphicPipelineStateUniquePtr(new GAPI::GraphicPipelineState(desc, name));
        multiThreadDevice->InitPipelineState(*resource.get());

        return resource;
    }
}
