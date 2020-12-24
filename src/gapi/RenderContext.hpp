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
        struct PresentOptions;

        // Todo thread safety?
        class RenderContext final : public Singleton<RenderContext>
        {
        public:
            RenderContext();
            ~RenderContext();

            Result Init(const PresentOptions& presentOptions);
            void Terminate();

            void Submit(const std::shared_ptr<CommandQueue>& commandQueue, const std::shared_ptr<CommandList>& CommandList);
            void Present();
            Result ResetDevice(const PresentOptions& presentOptions);
            Result ResetSwapChain(const std::shared_ptr<SwapChain>& swapchain, SwapChainDescription& description);

            std::shared_ptr<CommandList> CreateCommandList(const U8String& name) const;
            std::shared_ptr<CommandQueue> RenderContext::CreteCommandQueue(CommandQueueType type, const U8String& name) const;
            std::shared_ptr<Fence> RenderContext::CreateFence(uint64_t initialValue, const U8String& name = "") const;
            std::shared_ptr<Texture> CreateTexture(const TextureDescription& desc, Resource::BindFlags bindFlags, const U8String& name = "") const;
            std::shared_ptr<RenderTargetView> CreateRenderTargetView(const std::shared_ptr<Texture>& texture, const ResourceViewDescription& desc, const U8String& name = "") const;
            std::shared_ptr<SwapChain> CreateSwapchain(const SwapChainDescription& description, const U8String& name = "") const;

        private:
            Render::Result initDevice();
            Render::Result resetDevice(const PresentOptions& presentOptions);

        private:
            static constexpr int SubmissionThreadAheadFrames = 4;

            bool inited_ = false;

            std::array<std::unique_ptr<Threading::Event>, SubmissionThreadAheadFrames> presentEvents_;
            int presentIndex_ = 0;

            std::shared_ptr<Fence> fence_;
            std::unique_ptr<Submission> submission_;
        };
    }
}