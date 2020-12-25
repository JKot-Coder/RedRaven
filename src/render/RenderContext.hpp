#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"

#include "common/Singleton.hpp"

#include <array>

namespace OpenDemo
{
    namespace Common
    {
        namespace Threading
        {
            class Event;
        }
    }

    namespace Render
    {
        class Submission;

        // Todo thread safety?
        class RenderContext final : public Singleton<RenderContext>
        {
        public:
            RenderContext();
            ~RenderContext();

            GAPI::Result Init(const GAPI::PresentOptions& presentOptions);
            void Terminate();

            // void Submit(const std::shared_ptr<CommandQueue>& commandQueue, const std::shared_ptr<CommandList>& CommandList);
            void Present();
            GAPI::Result ResetDevice(const GAPI::PresentOptions& presentOptions);
            GAPI::Result ResetSwapChain(const std::shared_ptr<GAPI::SwapChain>& swapchain, GAPI::SwapChainDescription& description);

            std::shared_ptr<GAPI::CopyCommandList> CreateCopyCommandList(const U8String& name) const;
            std::shared_ptr<GAPI::ComputeCommandList> CreateComputeCommandList(const U8String& name) const;
            std::shared_ptr<GAPI::GraphicsCommandList> CreateGraphicsCommandList(const U8String& name) const;
            std::shared_ptr<GAPI::CommandQueue> RenderContext::CreteCommandQueue(GAPI::CommandQueueType type, const U8String& name) const;
            std::shared_ptr<GAPI::Fence> RenderContext::CreateFence(uint64_t initialValue, const U8String& name = "") const;
            std::shared_ptr<GAPI::Texture> CreateTexture(const GAPI::TextureDescription& desc, GAPI::Resource::BindFlags bindFlags, const U8String& name = "") const;
            std::shared_ptr<GAPI::RenderTargetView> CreateRenderTargetView(const std::shared_ptr<GAPI::Texture>& texture, const GAPI::ResourceViewDescription& desc, const U8String& name = "") const;
            std::shared_ptr<GAPI::SwapChain> CreateSwapchain(const GAPI::SwapChainDescription& description, const U8String& name = "") const;

        private:
            GAPI::Result initDevice();
            GAPI::Result resetDevice(const GAPI::PresentOptions& presentOptions);

        private:
            static constexpr int SubmissionThreadAheadFrames = 4;

            bool inited_ = false;

            std::array<uint64_t, SubmissionThreadAheadFrames> fenceValues_;
            uint32_t frameIndex_ = 0;

            std::shared_ptr<GAPI::Fence> fence_;
            std::unique_ptr<Submission> submission_;
        };
    }
}