#pragma once

#include "gapi/ForwardDeclarations.hpp"

#include "render/ResourcePointers.hpp"
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
        void ResizeSwapChain(GAPI::SwapChain* swapchain, uint32_t width, uint32_t height);

        void Compile(CommandEncoder& commandEncoder);

        void Submit(GAPI::CommandQueue* commandQueue, CommandEncoder& commandEncoder)
        {
            ASSERT(inited);
            ASSERT(commandQueue);

            submission.Submit(commandQueue, commandEncoder.GetCommandList());
        }

        CommandQueueUniquePtr CreateCommandQueue(GAPI::CommandQueueType type, const std::string& name) const;
        ShaderUniquePtr CreateShader(const GAPI::ShaderDesc& desc, const std::string& name) const;
        BufferUniquePtr CreateBuffer(const GAPI::GpuResourceDesc& desc, const GAPI::BufferData* initialData, const std::string& name = "") const;
        TextureUniquePtr CreateTexture(const GAPI::GpuResourceDesc& desc, const eastl::shared_ptr<Common::IDataBuffer>& initialData, const std::string& name);
        RenderTargetViewUniquePtr CreateRenderTargetView(GAPI::Texture& texture, const GAPI::GpuResourceViewDesc& desc) const;
        DepthStencilViewUniquePtr CreateDepthStencilView(GAPI::Texture& texture, const GAPI::GpuResourceViewDesc& desc) const;
        ShaderResourceViewUniquePtr CreateShaderResourceView(GAPI::GpuResource& gpuResource, const GAPI::GpuResourceViewDesc& desc) const;
        UnorderedAccessViewUniquePtr CreateUnorderedAccessView(GAPI::GpuResource& gpuResource, const GAPI::GpuResourceViewDesc& desc) const;
        SwapChainUniquePtr CreateSwapchain(const GAPI::SwapChainDesc& desc) const;
        TextureUniquePtr CreateSwapChainBackBuffer(GAPI::SwapChain& swapchain, const GAPI::GpuResourceDesc& desc, const std::string& name) const;
        GraphicPipelineStateUniquePtr CreatePipelineState(const GAPI::GraphicPipelineStateDesc& desc, const std::string& name) const;
        BindingGroupUniquePtr CreateBindingGroup(const GAPI::BindingGroupDesc& desc, const std::string& name) const;

        eastl::unique_ptr<CommandEncoder> CreateCommandEncoder(const std::string& name) const;
        eastl::unique_ptr<Render::Effect> CreateEffect(const std::string& name, EffectDesc&& effectDesc) const;

    private:
        bool inited = false;
        Submission submission;
        GAPI::Device::IMultiThreadDevice* multiThreadDevice = nullptr;
    };
}