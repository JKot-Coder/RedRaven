#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/GpuResource.hpp"

// TODO remove
#include "common/Singleton.hpp"
#include "render/Submission.hpp"

namespace OpenDemo
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

            static constexpr uint32_t MaxPossible = 0xFFFFFF;

            void Init();
            void Terminate();

            void Submit(const std::shared_ptr<GAPI::CommandQueue>& commandQueue, const std::shared_ptr<GAPI::CommandList>& CommandList);
            void Present(const std::shared_ptr<GAPI::SwapChain>& swapChain);
            void WaitForGpu(const std::shared_ptr<GAPI::CommandQueue>& commandQueue);
            void MoveToNextFrame(const std::shared_ptr<GAPI::CommandQueue>& commandQueue);
            void ResetSwapChain(const std::shared_ptr<GAPI::SwapChain>& swapchain, GAPI::SwapChainDescription& description);

            void ExecuteAsync(const Submission::CallbackFunction&& function);
            void ExecuteAwait(const Submission::CallbackFunction&& function);

            std::shared_ptr<GAPI::CpuResourceData> AllocateIntermediateTextureData(
                const GAPI::GpuResourceDescription& desc,
                GAPI::MemoryAllocationType memoryType,
                uint32_t firstSubresourceIndex = 0,
                uint32_t numSubresources = MaxPossible) const;

            std::shared_ptr<GAPI::CopyCommandList> CreateCopyCommandList(const U8String& name) const;
            std::shared_ptr<GAPI::ComputeCommandList> CreateComputeCommandList(const U8String& name) const;
            std::shared_ptr<GAPI::GraphicsCommandList> CreateGraphicsCommandList(const U8String& name) const;
            std::shared_ptr<GAPI::CommandQueue> CreteCommandQueue(GAPI::CommandQueueType type, const U8String& name) const;
            std::shared_ptr<GAPI::Fence> CreateFence(const U8String& name = "") const;
            std::shared_ptr<GAPI::Buffer> CreateBuffer(const GAPI::GpuResourceDescription& desc, GAPI::GpuResourceCpuAccess cpuAccess = GAPI::GpuResourceCpuAccess::None, const U8String& name = "") const;
            std::shared_ptr<GAPI::Texture> CreateTexture(const GAPI::GpuResourceDescription& desc, GAPI::GpuResourceCpuAccess cpuAccess = GAPI::GpuResourceCpuAccess::None, const U8String& name = "") const;
            std::shared_ptr<GAPI::Texture> CreateSwapChainBackBuffer(const std::shared_ptr<GAPI::SwapChain>& swapchain, uint32_t backBufferIndex, const GAPI::GpuResourceDescription& desc, const U8String& name = "") const;
            std::shared_ptr<GAPI::ShaderResourceView> CreateShaderResourceView(const std::shared_ptr<GAPI::GpuResource>& resource, const GAPI::GpuResourceViewDescription& desc) const;
            std::shared_ptr<GAPI::DepthStencilView> CreateDepthStencilView(const std::shared_ptr<GAPI::Texture>& texture, const GAPI::GpuResourceViewDescription& desc) const;
            std::shared_ptr<GAPI::RenderTargetView> CreateRenderTargetView(const std::shared_ptr<GAPI::Texture>& texture, const GAPI::GpuResourceViewDescription& desc) const;
            std::shared_ptr<GAPI::UnorderedAccessView> CreateUnorderedAccessView(const std::shared_ptr<GAPI::GpuResource>& resource, const GAPI::GpuResourceViewDescription& desc) const;
            std::shared_ptr<GAPI::SwapChain> CreateSwapchain(const GAPI::SwapChainDescription& description, const U8String& name = "") const;

            void ReleaseResource(GAPI::Object& resource) const;

        private:
            // TODO sync with GPU_MAX
            static constexpr int GpuFramesBuffered = 3;

            bool inited_ = false;
            uint32_t frameIndex_ = 0;

            std::shared_ptr<GAPI::Fence> fence_;
            std::unique_ptr<Submission> submission_;
        };
    }
}