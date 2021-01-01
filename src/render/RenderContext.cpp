#include "RenderContext.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/DeviceInterface.hpp"
#include "gapi/Fence.hpp"
#include "gapi/ResourceViews.hpp"
#include "gapi/Result.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "render/Submission.hpp"

#include "common/threading/Event.hpp"

namespace OpenDemo
{
    namespace Render
    {
        RenderContext::RenderContext()
            : submission_(new Submission())
        {
        }

        RenderContext::~RenderContext() { }

        GAPI::Result RenderContext::Init()
        {
            ASSERT(!inited_)

            submission_->Start();

            GAPI::Result result = GAPI::Result::Ok;
            if (!(result = initDevice()))
            {
                Log::Print::Error("Render device init failed.\n");
                return result;
            }

            inited_ = true;

            fence_ = CreateFence("Frame sync fence");
            if (!fence_)
            {
                inited_ = false;
                Log::Print::Error("Failed init fence.\n");
                return result;
            }

            return result;
        }

        void RenderContext::Terminate()
        {
            ASSERT(inited_)

            submission_->Terminate();
            inited_ = false;
        }

        void RenderContext::Submit(const std::shared_ptr<GAPI::CommandQueue>& commandQueue, const std::shared_ptr<GAPI::CommandList>& commandList)
        {
            ASSERT(inited_)

            submission_->Submit(commandQueue, commandList);
        }

        void RenderContext::Present(const std::shared_ptr<GAPI::SwapChain>& swapChain)
        {
            ASSERT(inited_)

            submission_->ExecuteAsync([&swapChain](GAPI::Device& device) {
                const auto result = device.Present(swapChain);
                return result;
            });
        }

        GAPI::Result RenderContext::MoveToNextFrame(const std::shared_ptr<GAPI::CommandQueue>& commandQueue)
        {
            ASSERT(inited_)

            static uint32_t submissionFrame = 0;

            submission_->ExecuteAsync([this, &commandQueue](GAPI::Device& device) {
                GAPI::Result result = GAPI::Result::Ok;

                submissionFrame++;
                const auto currentFenceValue = fence_->GetCpuValue();

                // Schedule a Signal command in the queue.
                if (!(result = fence_->Signal(commandQueue)))
                    return result;

                if (currentFenceValue >= SubmissionThreadAheadFrames)
                {
                    // GPU ahead. Throttle cpu.
                    // TODO SubmissionThreadAheadFrames rename/replace
                    if (!(fence_->SyncCPU(currentFenceValue - SubmissionThreadAheadFrames, INFINITE)))
                        return result;
                }

                return result;
            });

            return GAPI::Result::Ok;
        }

        void RenderContext::ExecuteAsync(const Submission::CallbackFunction&& function)
        {
            ASSERT(inited_);

            // Todo optimize
            submission_->ExecuteAsync([function](GAPI::Device& device) { return function(device); });
        }

        GAPI::Result RenderContext::ExecuteAwait(const Submission::CallbackFunction&& function)
        {
            ASSERT(inited_);

            // Todo optimize
            return submission_->ExecuteAwait([function](GAPI::Device& device) { return function(device); });
        }

        GAPI::Result RenderContext::ResetSwapChain(const std::shared_ptr<GAPI::SwapChain>& swapchain, GAPI::SwapChainDescription& description)
        {
            ASSERT(inited_);

            return submission_->ExecuteAwait([&swapchain, &description](GAPI::Device& device) {
                GAPI::Result result = GAPI::Result::Ok;

                result = device.WaitForGpu();
                if (!result)
                    return result;

                result = swapchain->Reset(description);
                if (!result)
                    return result;

                return result;
            });
        }

        GAPI::CopyCommandList::SharedPtr RenderContext::CreateCopyCommandList(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::CopyCommandList::Create(name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::ComputeCommandList::SharedPtr RenderContext::CreateComputeCommandList(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::ComputeCommandList::Create(name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::GraphicsCommandList::SharedPtr RenderContext::CreateGraphicsCommandList(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::GraphicsCommandList::Create(name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::CommandQueue::SharedPtr RenderContext::CreteCommandQueue(GAPI::CommandQueueType type, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::CommandQueue::Create(type, name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::Fence::SharedPtr RenderContext::CreateFence(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::Fence::Create(name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::Texture::SharedPtr RenderContext::CreateTexture(const GAPI::TextureDescription& desc, GAPI::Texture::BindFlags bindFlags, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::Texture::Create(desc, bindFlags, name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::Texture::SharedPtr RenderContext::CreateSwapChainBackBuffer(const std::shared_ptr<GAPI::SwapChain>& swapchain, uint32_t backBufferIndex, const GAPI::TextureDescription& desc, GAPI::Texture::BindFlags bindFlags, const U8String& name) const
        {
            ASSERT(inited_)
            ASSERT(swapchain)

            auto& resource = GAPI::Texture::Create(desc, bindFlags, name);
            if (!swapchain->InitBackBufferTexture(backBufferIndex, resource))
                resource = nullptr;

            return resource;
        }

        GAPI::RenderTargetView::SharedPtr RenderContext::CreateRenderTargetView(const GAPI::Texture::SharedPtr& texture, const GAPI::ResourceViewDescription& desc, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::RenderTargetView::Create(texture, desc, name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::SwapChain::SharedPtr RenderContext::CreateSwapchain(const GAPI::SwapChainDescription& description, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::SwapChain::Create(description, name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::Result RenderContext::initDevice()
        {
            return submission_->ExecuteAwait([](GAPI::Device& device) {
                return device.Init();
            });
        }
    }
}