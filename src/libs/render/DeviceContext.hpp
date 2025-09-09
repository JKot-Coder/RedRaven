#pragma once

#include "gapi/ForwardDeclarations.hpp"

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
    class GraphicsCommandContext;
    class Effect;
    struct EffectDesc;

    class DeviceContext : public Common::Singleton<DeviceContext>
    {
    public:
        using SharedPtr = eastl::shared_ptr<DeviceContext>;
        using SharedConstPtr = eastl::shared_ptr<const DeviceContext>;

        DeviceContext();
        ~DeviceContext();

        void Init(const GAPI::DeviceDesc& desc);

        void Present(GAPI::SwapChain* swapChain);
        void MoveToNextFrame(uint64_t frameIndex);
        void ResizeSwapChain(GAPI::SwapChain* swapchain, uint32_t width, uint32_t height);

        void Compile(GAPI::CommandList2& commandList);

        eastl::unique_ptr<GAPI::CommandQueue> CreateCommandQueue(GAPI::CommandQueueType type, const std::string& name) const;
        eastl::unique_ptr<Render::GraphicsCommandContext> CreateGraphicsCommandContext(const std::string& name) const;
        eastl::unique_ptr<Render::Effect> CreateEffect(const std::string& name, EffectDesc&& effectDesc) const;
        eastl::unique_ptr<GAPI::Shader> CreateShader(const GAPI::ShaderDesc& desc, const std::string& name) const;
        eastl::shared_ptr<GAPI::Texture> CreateTexture(const GAPI::GpuResourceDesc& desc, const eastl::shared_ptr<Common::IDataBuffer>& initialData, const std::string& name);
        eastl::unique_ptr<GAPI::RenderTargetView> CreateRenderTargetView(const eastl::shared_ptr<GAPI::Texture>& texture, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::DepthStencilView> CreateDepthStencilView(const eastl::shared_ptr<GAPI::Texture>& texture, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::ShaderResourceView> CreateShaderResourceView(const eastl::shared_ptr<GAPI::GpuResource>& gpuResource, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::UnorderedAccessView> CreateUnorderedAccessView(const eastl::shared_ptr<GAPI::GpuResource>& gpuResource, const GAPI::GpuResourceViewDesc& desc) const;
        eastl::unique_ptr<GAPI::SwapChain> CreateSwapchain(const GAPI::SwapChainDesc& desc) const;
        eastl::shared_ptr<GAPI::Texture> CreateSwapChainBackBuffer(const GAPI::SwapChain* swapchain, uint32_t backBufferIndex, const GAPI::GpuResourceDesc& desc, const std::string& name) const;
        eastl::shared_ptr<GAPI::Texture> CreateSwapChainDepthBuffer(const GAPI::SwapChain* swapchain, const GAPI::GpuResourceDesc& desc) const;
        eastl::unique_ptr<GAPI::GraphicPipelineState> CreatePipelineState(const GAPI::GraphicPipelineStateDesc& desc, const std::string& name) const;


    private:
        bool inited = false;
        eastl::unique_ptr<GAPI::Device> device;
    };
}