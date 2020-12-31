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

        GAPI::Result RenderContext::Init(const GAPI::PresentOptions& presentOptions)
        {
            ASSERT(!inited_)

            submission_->Start();

            GAPI::Result result = GAPI::Result::Ok;
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
        }

        GAPI::Result RenderContext::MoveToNextFrame(const std::shared_ptr<GAPI::CommandQueue>& commandQueue)
        {
            ASSERT(inited_)

            GAPI::Result result = GAPI::Result::Ok;

            // Schedule a Signal command in the queue.
            if (!(result = fence_->Signal(commandQueue)))
                return result;

            if (fence_->GetCpuValue() >= SubmissionThreadAheadFrames)
            {
                // GPU ahead. Throttle cpu.
                if (!(fence_->SyncCPU(fence_->GetCpuValue() - SubmissionThreadAheadFrames, INFINITE)))
                    return result;
            }

            return result;
        }

        GAPI::Result RenderContext::ResetDevice(const GAPI::PresentOptions& presentOptions)
        {
            ASSERT(inited_)

            return resetDevice(presentOptions);
        }

        GAPI::Result RenderContext::ResetSwapChain(const std::shared_ptr<GAPI::SwapChain>& swapchain, GAPI::SwapChainDescription& description)
        {
            ASSERT(inited_)

            return submission_->ExecuteAwait([&swapchain, &description](GAPI::Device& device) {
                return device.ResetSwapchain(swapchain, description);
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

        GAPI::Result RenderContext::resetDevice(const GAPI::PresentOptions& presentOptions)
        {
            return submission_->ExecuteAwait([&presentOptions](GAPI::Device& device) {
                return device.Reset(presentOptions);
            });
        }
    }
}