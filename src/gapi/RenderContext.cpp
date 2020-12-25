#include "RenderContext.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/DeviceInterface.hpp"
#include "gapi/Fence.hpp"
#include "gapi/ResourceViews.hpp"
#include "gapi/Result.hpp"
#include "gapi/Submission.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "common/threading/Event.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        RenderContext::RenderContext()
            : submission_(new Submission())
        {
        }

        RenderContext::~RenderContext() { }

        Result RenderContext::Init(const GAPI::PresentOptions& presentOptions)
        {
            ASSERT(!inited_)

            submission_->Start();

            Result result = Result::Ok;
            if (!(result = initDevice()))
            {
                Log::Print::Error("Render device init failed.\n");
                return result;
            }

            ;
            if (!(result = resetDevice(presentOptions)))
            {
                Log::Print::Error("Render device reset failed.\n");
                return result;
            }

            inited_ = true;

            fence_ = CreateFence(0, "Frame sync fence");
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
        /*
        void RenderContext::Submit(const CommandQueue::SharedPtr& commandQueue, const CommandList::SharedPtr& commandList)
        {
            ASSERT(inited_)

            submission_->Submit(commandQueue, commandList);
        }*/

        void RenderContext::Present()
        {
            ASSERT(inited_)

            submission_->ExecuteAsync([](GAPI::Device& device) {
                const auto result = device.Present();
                return result;
            });

            // Schedule a Signal command in the queue.
            const uint64_t currentFenceValue = fence_->GetCpuValue();
            // fence_->Signal();

            // This will limit count of main thread frames ahead.
            frameIndex_ = (frameIndex_++ % SubmissionThreadAheadFrames);
            fence_->SyncCPU(fenceValues_[frameIndex_], INFINITE);

            // Set the fence value for the next time.
            fenceValues_[frameIndex_] = currentFenceValue + 1;
        }

        GAPI::Result RenderContext::ResetDevice(const PresentOptions& presentOptions)
        {
            ASSERT(inited_)

            return resetDevice(presentOptions);
        }

        Result RenderContext::ResetSwapChain(const std::shared_ptr<SwapChain>& swapchain, SwapChainDescription& description)
        {
            ASSERT(inited_)

            return submission_->ExecuteAwait([&swapchain, &description](GAPI::Device& device) {
                return device.ResetSwapchain(swapchain, description);
            });
        }

        CopyCommandList::SharedPtr RenderContext::CreateCopyCommandList(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = CopyCommandList::Create(name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        ComputeCommandList::SharedPtr RenderContext::CreateComputeCommandList(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = ComputeCommandList::Create(name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GraphicsCommandList::SharedPtr RenderContext::CreateGraphicsCommandList(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GraphicsCommandList::Create(name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        CommandQueue::SharedPtr RenderContext::CreteCommandQueue(CommandQueueType type, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = CommandQueue::Create(type, name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        Fence::SharedPtr RenderContext::CreateFence(uint64_t initialValue, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = Fence::Create(name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource, initialValue))
                resource = nullptr;

            return resource;
        }

        Texture::SharedPtr RenderContext::CreateTexture(const TextureDescription& desc, Texture::BindFlags bindFlags, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = Texture::Create(desc, bindFlags, name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        RenderTargetView::SharedPtr RenderContext::CreateRenderTargetView(const Texture::SharedPtr& texture, const ResourceViewDescription& desc, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = RenderTargetView::Create(texture, desc, name);
            if (!submission_->GetMultiThreadDeviceInterface().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        SwapChain::SharedPtr RenderContext::CreateSwapchain(const SwapChainDescription& description, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = SwapChain::Create(description, name);
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

        GAPI::Result RenderContext::resetDevice(const PresentOptions& presentOptions)
        {
            return submission_->ExecuteAwait([&presentOptions](GAPI::Device& device) {
                return device.Reset(presentOptions);
            });
        }
    }
}