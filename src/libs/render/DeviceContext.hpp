#pragma once

#include "gapi/ForwardDeclarations.hpp"

#include "render/CommandEncoder.hpp"
#include "render/Submission.hpp"

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
    class CommandEncoder;
    class RenderPassEncoder;
    class Effect;
    class SwapChain;
    struct EffectDesc;

    class DeviceContext : public Common::Singleton<DeviceContext>
    {
    public:
        DeviceContext();
        ~DeviceContext();

        [[nodiscard]] bool Init(const GAPI::DeviceDesc& desc);
        void Terminate();

        void Present(Render::SwapChain& swapChain);
        void MoveToNextFrame(uint64_t frameIndex);
        void ResizeSwapChain(Render::SwapChain* swapchain, uint32_t width, uint32_t height);

        void Compile(CommandEncoder& commandEncoder);

        void Submit(GAPI::CommandQueue* commandQueue, CommandEncoder& commandEncoder)
        {
            ASSERT(inited);
            ASSERT(commandQueue);

            submission.Submit(commandQueue, commandEncoder.GetCommandList());
        }

        eastl::unique_ptr<GAPI::CommandQueue> CreateCommandQueue(GAPI::CommandQueueType type, const std::string& name) const;
        eastl::unique_ptr<GAPI::Shader> CreateShader(const GAPI::ShaderDesc& desc, const std::string& name) const;
        eastl::unique_ptr<GAPI::Buffer> CreateBuffer(const GAPI::GpuResourceDesc& desc, const GAPI::BufferData* initialData, const std::string& name = "") const;
        eastl::unique_ptr<GAPI::Texture> CreateTexture(const GAPI::GpuResourceDesc& desc, const eastl::shared_ptr<Common::IDataBuffer>& initialData, const std::string& name);
        eastl::unique_ptr<GAPI::RenderTargetView> CreateRenderTargetView(GAPI::Texture& texture, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::DepthStencilView> CreateDepthStencilView(GAPI::Texture& texture, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::ShaderResourceView> CreateShaderResourceView(GAPI::GpuResource& gpuResource, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::UnorderedAccessView> CreateUnorderedAccessView(GAPI::GpuResource& gpuResource, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::SwapChain> CreateSwapchain(const GAPI::SwapChainDesc& desc) const;
        eastl::unique_ptr<GAPI::Texture> CreateSwapChainBackBuffer(GAPI::SwapChain& swapchain, const GAPI::GpuResourceDesc& desc, const std::string& name) const;
        eastl::unique_ptr<GAPI::GraphicPipelineState> CreatePipelineState(const GAPI::GraphicPipelineStateDesc& desc, const std::string& name) const;
        eastl::unique_ptr<GAPI::BindingGroupLayout> CreateBindingGroupLayout(const GAPI::BindingGroupLayoutDesc& desc, const std::string& name) const;
        eastl::unique_ptr<GAPI::BindingGroup> CreateBindingGroup(const GAPI::BindingGroupDesc& desc, const std::string& name) const;

        eastl::unique_ptr<CommandEncoder> CreateCommandEncoder(const std::string& name) const;
        eastl::unique_ptr<Render::Effect> CreateEffect(const std::string& name, EffectDesc&& effectDesc) const;

    private:
        bool inited = false;
        Submission submission;
        GAPI::Device::IMultiThreadDevice* multiThreadDevice = nullptr;
    };
}