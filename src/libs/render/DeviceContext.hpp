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
    struct EffectDesc;

    class DeviceContext : public Common::Singleton<DeviceContext>
    {
    public:
        DeviceContext();
        ~DeviceContext();

        [[nodiscard]] bool Init(const GAPI::DeviceDesc& desc);
        void Terminate();

        void Present(GAPI::SwapChain* swapChain);
        void MoveToNextFrame(uint64_t frameIndex);
        void ResizeSwapChain(GAPI::SwapChain* swapchain, uint32_t width, uint32_t height);

        void Compile(CommandEncoder& commandEncoder);

        void Submit(GAPI::CommandQueue* commandQueue, CommandEncoder& commandEncoder)
        {
            ASSERT(inited);
            ASSERT(commandQueue);

            submission.Submit(commandQueue, commandEncoder.GetCommandList());
        }

        eastl::unique_ptr<GAPI::CommandQueue> CreateCommandQueue(GAPI::CommandQueueType type, const std::string& name) const;
        eastl::unique_ptr<Render::CommandEncoder> CreateCommandEncoder(const std::string& name) const;
        eastl::unique_ptr<Render::Effect> CreateEffect(const std::string& name, EffectDesc&& effectDesc) const;
        eastl::unique_ptr<GAPI::Shader> CreateShader(const GAPI::ShaderDesc& desc, const std::string& name) const;
        eastl::shared_ptr<GAPI::Buffer> CreateBuffer(const GAPI::GpuResourceDesc& desc, const GAPI::BufferData* initialData, const std::string& name = "") const;
        eastl::shared_ptr<GAPI::Texture> CreateTexture(const GAPI::GpuResourceDesc& desc, const eastl::shared_ptr<Common::IDataBuffer>& initialData, const std::string& name);
        eastl::unique_ptr<GAPI::RenderTargetView> CreateRenderTargetView(const eastl::shared_ptr<GAPI::Texture>& texture, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::DepthStencilView> CreateDepthStencilView(const eastl::shared_ptr<GAPI::Texture>& texture, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::ShaderResourceView> CreateShaderResourceView(const eastl::shared_ptr<GAPI::GpuResource>& gpuResource, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::UnorderedAccessView> CreateUnorderedAccessView(const eastl::shared_ptr<GAPI::GpuResource>& gpuResource, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::SwapChain> CreateSwapchain(const GAPI::SwapChainDesc& desc) const;
        eastl::shared_ptr<GAPI::Texture> CreateSwapChainBackBuffer(const GAPI::SwapChain* swapchain, const GAPI::GpuResourceDesc& desc, const std::string& name) const;
        eastl::unique_ptr<GAPI::GraphicPipelineState> CreatePipelineState(const GAPI::GraphicPipelineStateDesc& desc, const std::string& name) const;
        eastl::unique_ptr<GAPI::BindingGroup> CreateBindingGroup(const GAPI::BindingGroupDesc& desc, const std::string& name) const;

    private:
        bool inited = false;
        Submission submission;
        GAPI::Device::IMultiThreadDevice* multiThreadDevice = nullptr;
    };
}