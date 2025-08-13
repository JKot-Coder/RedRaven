#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/GpuResource.hpp"
#include "gapi/Limits.hpp"

#include "render/Submission.hpp"

#include "common/Singleton.hpp"
#include "common/threading/Event.hpp"

namespace RR
{
    namespace Render
    {
        class Submission;

        // Todo thread safety?
        class DeviceContext final : public Singleton<DeviceContext>
        {
        public:
            DeviceContext();
            ~DeviceContext();

            void Init(uint32_t gpuFramesBuffered, uint32_t submitFramesBuffered);
            void Terminate();

            void Submit(const eastl::shared_ptr<GAPI::CommandQueue>& commandQueue, const eastl::shared_ptr<GAPI::CommandList>& CommandList);
            void Present(const eastl::shared_ptr<GAPI::SwapChain>& swapChain);
            void WaitForGpu(const eastl::shared_ptr<GAPI::CommandQueue>& commandQueue);
            void MoveToNextFrame(const eastl::shared_ptr<GAPI::CommandQueue>& commandQueue);
            void ResetSwapChain(const eastl::shared_ptr<GAPI::SwapChain>& swapchain, GAPI::SwapChainDescription& description);

            void ExecuteAsync(const Submission::CallbackFunction&& function);
            void ExecuteAwait(const Submission::CallbackFunction&& function);

            GAPI::GpuResourceFootprint GetResourceFootprint(const GAPI::GpuResourceDescription& desc) const;

            eastl::shared_ptr<GAPI::Buffer> CreateBuffer(const GAPI::GpuResourceDescription& desc, IDataBuffer::SharedPtr initialData = nullptr, const std::string& name = "") const;
            eastl::shared_ptr<GAPI::CommandQueue> CreteCommandQueue(GAPI::CommandQueueType type, const std::string& name) const;
            eastl::shared_ptr<GAPI::ComputeCommandList> CreateComputeCommandList(const std::string& name) const;
            eastl::shared_ptr<GAPI::CopyCommandList> CreateCopyCommandList(const std::string& name) const;
            eastl::shared_ptr<GAPI::DepthStencilView> CreateDepthStencilView(const eastl::shared_ptr<GAPI::Texture>& texture, const GAPI::GpuResourceViewDescription& desc) const;
            eastl::shared_ptr<GAPI::Fence> CreateFence(const std::string& name = "") const;
            eastl::shared_ptr<GAPI::Framebuffer> CreateFramebuffer(const GAPI::FramebufferDesc& desc) const;
            eastl::shared_ptr<GAPI::GraphicsCommandList> CreateGraphicsCommandList(const std::string& name) const;
            eastl::shared_ptr<GAPI::RenderTargetView> CreateRenderTargetView(const eastl::shared_ptr<GAPI::Texture>& texture, const GAPI::GpuResourceViewDescription& desc) const;
            eastl::shared_ptr<GAPI::ShaderResourceView> CreateShaderResourceView(const eastl::shared_ptr<GAPI::GpuResource>& resource,
                                                                               const GAPI::GpuResourceViewDescription& desc) const;
            eastl::shared_ptr<GAPI::SwapChain> CreateSwapchain(const GAPI::SwapChainDescription& description, const std::string& name = "") const;
            eastl::shared_ptr<GAPI::Texture> CreateSwapChainBackBuffer(const eastl::shared_ptr<GAPI::SwapChain>& swapchain,
                                                                     uint32_t backBufferIndex,
                                                                     const GAPI::GpuResourceDescription& desc,
                                                                     const std::string& name = "") const;
            eastl::shared_ptr<GAPI::Texture> CreateTexture(const GAPI::GpuResourceDescription& desc, IDataBuffer::SharedPtr initialData = nullptr, const std::string& name = "") const;
            eastl::shared_ptr<GAPI::UnorderedAccessView> CreateUnorderedAccessView(const eastl::shared_ptr<GAPI::GpuResource>& resource, const GAPI::GpuResourceViewDescription& desc) const;

        private:
            uint32_t gpuFramesBuffered_ = 1;
            uint32_t submitFramesBuffered_ = 1;
            bool inited_ = false;

            std::array<Common::Threading::Event, GAPI::MAX_GPU_FRAMES_BUFFERED> moveToNextFrameEvents_ {{{false, true}, {false, true}, {false, true}}};
            eastl::shared_ptr<GAPI::Fence> fence_;
            std::unique_ptr<Submission> submission_;
        };
    }
}