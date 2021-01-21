#include "RenderContext.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Fence.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/IDevice.hpp"
#include "gapi/Result.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "render/Submission.hpp"

#include "common/threading/Event.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace
        {
            template <typename T>
            struct GPIObjectsDeleter
            {
                void operator()(T* p)
                {
                    static_assert(std::is_convertible<T*, GAPI::Object*>::value, "T shoud be convertable to GAPI::Object");

                    ASSERT(p);

                    const auto& instance = RenderContext::Instance();
                    instance.ReleaseResource(*static_cast<GAPI::Object*>(p));

                    delete p;
                }
            };
        }

        RenderContext::RenderContext()
            : submission_(new Submission()),
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
            // Force debug 
            debugMode = GAPI::Device::DebugMode::Debug;
#endif

            GAPI::Device::Description description(GpuFramesBuffered, debugMode);

            // Init Device
            result = submission_->ExecuteAwait([&description](GAPI::Device& device) {
                return device.Init(description);
            });

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

            //Release resources before termination;
            fence_.reset();

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

            submission_->ExecuteAsync([swapChain](GAPI::Device& device) {
                const auto result = device.Present(swapChain);
                return result;
            });
        }

        void RenderContext::MoveToNextFrame(const std::shared_ptr<GAPI::CommandQueue>& commandQueue)
        {
            ASSERT(inited_)

            static uint32_t submissionFrame = 0;

            submission_->ExecuteAsync([fence = fence_, commandQueue](GAPI::Device& device) {
                GAPI::Result result = GAPI::Result::Ok;

                submissionFrame++;

                // Schedule a Signal command in the queue.
                if (!(result = fence->Signal(commandQueue)))
                    return result;

                if (fence->GetCpuValue() >= GpuFramesBuffered)
                {
                    uint64_t syncFenceValue = fence->GetCpuValue() - GpuFramesBuffered;

                    // We shoud had at least one completed frame in ringbuffer.
                    syncFenceValue++;

                    // Throttle cpu if gpu behind
                    if (!(result = fence->SyncCPU(syncFenceValue, INFINITE)))
                        return result;
                }

                if (!(result = device.MoveToNextFrame()))
                    return result;

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

        void RenderContext::ResetSwapChain(const std::shared_ptr<GAPI::SwapChain>& swapchain, GAPI::SwapChainDescription& description)
        {
            ASSERT(inited_);

            auto result = submission_->ExecuteAwait([&swapchain, &description](GAPI::Device& device) {
                GAPI::Result result = GAPI::Result::Ok;

                if (!(result = device.WaitForGpu()))
                    return result;

                if (!(result = swapchain->Reset(description)))
                    return result;

                return result;
            });

            ASSERT(result);
        }

        GAPI::CopyCommandList::SharedPtr RenderContext::CreateCopyCommandList(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::CopyCommandList::Create(name, GPIObjectsDeleter<GAPI::CopyCommandList>());
            if (!submission_->GetIMultiThreadDevice().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::ComputeCommandList::SharedPtr RenderContext::CreateComputeCommandList(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::ComputeCommandList::Create(name, GPIObjectsDeleter<GAPI::ComputeCommandList>());
            if (!submission_->GetIMultiThreadDevice().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::GraphicsCommandList::SharedPtr RenderContext::CreateGraphicsCommandList(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::GraphicsCommandList::Create(name, GPIObjectsDeleter<GAPI::GraphicsCommandList>());
            if (!submission_->GetIMultiThreadDevice().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::CommandQueue::SharedPtr RenderContext::CreteCommandQueue(GAPI::CommandQueueType type, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::CommandQueue::Create(type, name, GPIObjectsDeleter<GAPI::CommandQueue>());
            if (!submission_->GetIMultiThreadDevice().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::Fence::SharedPtr RenderContext::CreateFence(const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::Fence::Create(name, GPIObjectsDeleter<GAPI::Fence>());
            if (!submission_->GetIMultiThreadDevice().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::Texture::SharedPtr RenderContext::CreateTexture(const GAPI::TextureDescription& desc, GAPI::GpuResourceBindFlags bindFlags, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::Texture::Create(desc, bindFlags, name, GPIObjectsDeleter<GAPI::Texture>());
            if (!submission_->GetIMultiThreadDevice().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::Texture::SharedPtr RenderContext::CreateSwapChainBackBuffer(const std::shared_ptr<GAPI::SwapChain>& swapchain, uint32_t backBufferIndex, const GAPI::TextureDescription& desc, GAPI::GpuResourceBindFlags bindFlags, const U8String& name) const
        {
            ASSERT(inited_)
            ASSERT(swapchain)

            auto& resource = GAPI::Texture::Create(desc, bindFlags, name, GPIObjectsDeleter<GAPI::Texture>());
            if (!swapchain->InitBackBufferTexture(backBufferIndex, resource))
                resource = nullptr;

            return resource;
        }

        GAPI::ShaderResourceView::SharedPtr RenderContext::CreateShaderResourceView(
            const std::shared_ptr<GAPI::GpuResource>& gpuResource,
            const GAPI::GpuResourceViewDescription& desc,
            const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::ShaderResourceView::Create(gpuResource, desc, name, GPIObjectsDeleter<GAPI::ShaderResourceView>());
            if (!submission_->GetIMultiThreadDevice().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::DepthStencilView::SharedPtr RenderContext::CreateDepthStencilView(
            const GAPI::Texture::SharedPtr& texture,
            const GAPI::GpuResourceViewDescription&
                desc,
            const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::DepthStencilView::Create(texture, desc, name, GPIObjectsDeleter<GAPI::DepthStencilView>());
            if (!submission_->GetIMultiThreadDevice().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::RenderTargetView::SharedPtr RenderContext::CreateRenderTargetView(
            const GAPI::Texture::SharedPtr& texture,
            const GAPI::GpuResourceViewDescription& desc,
            const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::RenderTargetView::Create(texture, desc, name, GPIObjectsDeleter<GAPI::RenderTargetView>());
            if (!submission_->GetIMultiThreadDevice().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::UnorderedAccessView::SharedPtr RenderContext::CreateUnorderedAccessView(
            const std::shared_ptr<GAPI::GpuResource>& gpuResource,
            const GAPI::GpuResourceViewDescription& desc,
            const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::UnorderedAccessView::Create(gpuResource, desc, name, GPIObjectsDeleter<GAPI::UnorderedAccessView>());
            if (!submission_->GetIMultiThreadDevice().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        GAPI::SwapChain::SharedPtr RenderContext::CreateSwapchain(const GAPI::SwapChainDescription& description, const U8String& name) const
        {
            ASSERT(inited_);

            auto& resource = GAPI::SwapChain::Create(description, name, GPIObjectsDeleter<GAPI::SwapChain>());
            if (!submission_->GetIMultiThreadDevice().lock()->InitResource(resource))
                resource = nullptr;

            return resource;
        }

        void RenderContext::ReleaseResource(GAPI::Object& resource) const
        {
            ASSERT(inited_);

            if (!inited_) // Leaked resource release. We can't do nothing with it.
                return;

            submission_->GetIMultiThreadDevice().lock()->ReleaseResource(resource);
        }
    }
}