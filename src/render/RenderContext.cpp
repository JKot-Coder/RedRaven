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
            ASSERT(!inited_);

            submission_->Start();

            GAPI::Result result = GAPI::Result::Ok;

            auto debugMode = GAPI::Device::DebugMode::Retail;

#ifdef DEBUG
            debugMode = GAPI::Device::DebugMode::Debug;
#endif

            GAPI::Device::Description description(GpuFramesBuffered, debugMode);
           
            result = initDevice(description);
            if (!result)
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

        void RenderContext::MoveToNextFrame(const std::shared_ptr<GAPI::CommandQueue>& commandQueue)
        {
            ASSERT(inited_)

            static uint32_t submissionFrame = 0;

            submission_->ExecuteAsync([this, &commandQueue](GAPI::Device& device) {
                GAPI::Result result = GAPI::Result::Ok;

                submissionFrame++;

                // Schedule a Signal command in the queue.
                if (!(result = fence_->Signal(commandQueue)))
                    return result;

                if (fence_->GetCpuValue() >= GpuFramesBuffered)
                {
                    uint64_t syncFenceValue = fence_->GetCpuValue() - GpuFramesBuffered;

                    // We shoud had at least one completed frame in ringbuffer.
                    syncFenceValue++;

                    // Throttle cpu if gpu behind
                    if (!(fence_->SyncCPU(syncFenceValue, INFINITE)))
                        return result;
                }

                return result;
            });

            // Todo throttle main thread?
        }

        void RenderContext::ExecuteAsync(const Submission::CallbackFunction&& function)
        {
            ASSERT(inited_);

            submission_->ExecuteAsync(std::move(function));
        }

        GAPI::Result RenderContext::ExecuteAwait(const Submission::CallbackFunction&& function)
        {
            ASSERT(inited_);

            return submission_->ExecuteAwait(std::move(function));
        }

        void  RenderContext::ResetSwapChain(const std::shared_ptr<GAPI::SwapChain>& swapchain, GAPI::SwapChainDescription& description)
        {
            ASSERT(inited_);

            auto result = submission_->ExecuteAwait([&swapchain, &description](GAPI::Device& device) {
                GAPI::Result result = GAPI::Result::Ok;

                result = swapchain->Reset(description);
                if (!result)
                    return result;

                return result;
            });

            ASSERT(result);
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

        GAPI::Texture::SharedPtr RenderContext::CreateTexture(const GAPI::TextureDescription& desc, GAPI::ResourceBindFlags bindFlags, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::Texture::Create(desc, bindFlags, name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::Texture::SharedPtr RenderContext::CreateSwapChainBackBuffer(const std::shared_ptr<GAPI::SwapChain>& swapchain, uint32_t backBufferIndex, const GAPI::TextureDescription& desc, GAPI::ResourceBindFlags bindFlags, const U8String& name) const
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

        GAPI::Result RenderContext::initDevice(const GAPI::Device::Description& description)
        {
            return submission_->ExecuteAwait([&description](GAPI::Device& device) {
                return device.Init(description);
            });
        }
    }
}