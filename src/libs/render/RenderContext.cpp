#include "RenderContext.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Device.hpp"
#include "gapi/Fence.hpp"
#include "gapi/GpuResourceViews.hpp"
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
            : submission_(new Submission())
        {
        }

        RenderContext::~RenderContext() { }

        void RenderContext::Init()
        {
            ASSERT(!inited_);

            submission_->Start();

            auto debugMode = GAPI::Device::DebugMode::Retail;

#ifdef DEBUG
            // Force debug
            debugMode = GAPI::Device::DebugMode::Debug;
#endif

            GAPI::Device::Description description(GpuFramesBuffered, debugMode);

            // Init Device
            submission_->ExecuteAwait([&description](GAPI::Device& device) {
                if (!device.Init(description))
                    LOG_FATAL("Can't init device");
            });

            inited_ = true;

            fence_ = CreateFence("Frame sync fence");
        }

        void RenderContext::Terminate()
        {
            ASSERT(inited_);

            //Release resources before termination;
            fence_.reset();

            submission_->Terminate();
            inited_ = false;
        }

        void RenderContext::Submit(const std::shared_ptr<GAPI::CommandQueue>& commandQueue, const std::shared_ptr<GAPI::CommandList>& commandList)
        {
            ASSERT(inited_);

            submission_->Submit(commandQueue, commandList);
        }

        void RenderContext::Present(const std::shared_ptr<GAPI::SwapChain>& swapChain)
        {
            ASSERT(inited_);

            submission_->ExecuteAsync([swapChain](GAPI::Device& device) {
                return device.Present(swapChain);
            });
        }

        void RenderContext::WaitForGpu()
        {
            ASSERT(inited_);

            submission_->ExecuteAwait([](GAPI::Device& device) {
                return device.WaitForGpu();
            });
        }

        void RenderContext::MoveToNextFrame(const std::shared_ptr<GAPI::CommandQueue>& commandQueue)
        {
            ASSERT(inited_);

            static uint32_t submissionFrame = 0;

            submission_->ExecuteAsync([fence = fence_, commandQueue](GAPI::Device& device) {
                submissionFrame++;

                // Schedule a Signal command in the queue.
                fence->Signal(commandQueue);

                if (fence->GetCpuValue() >= GpuFramesBuffered)
                {
                    uint64_t syncFenceValue = fence->GetCpuValue() - GpuFramesBuffered;

                    // We shoud had at least one completed frame in ringbuffer.
                    syncFenceValue++;

                    // Throttle cpu if gpu behind
                    fence->SyncCPU(syncFenceValue, INFINITE);
                }

                device.MoveToNextFrame();
            });

            // Todo throttle main thread?
        }

        void RenderContext::ExecuteAsync(const Submission::CallbackFunction&& function)
        {
            ASSERT(inited_);

            submission_->ExecuteAsync(std::move(function));
        }

        void RenderContext::ExecuteAwait(const Submission::CallbackFunction&& function)
        {
            ASSERT(inited_);

            submission_->ExecuteAwait(std::move(function));
        }

        void RenderContext::ResetSwapChain(const std::shared_ptr<GAPI::SwapChain>& swapchain, GAPI::SwapChainDescription& description)
        {
            ASSERT(inited_);

            submission_->ExecuteAwait([&swapchain, &description](GAPI::Device& device) {
                device.WaitForGpu();
                swapchain->Reset(description);
            });
        }

        std::shared_ptr<GAPI::IntermediateMemory> RenderContext::AllocateIntermediateTextureData(
            const GAPI::TextureDescription& desc,
            GAPI::IntermediateMemoryType memoryType,
            uint32_t firstSubresourceIndex,
            uint32_t numSubresources) const
        {
            ASSERT(inited_);

            return submission_->GetIMultiThreadDevice().lock()->AllocateIntermediateTextureData(desc, memoryType, firstSubresourceIndex, numSubresources);
        }

        GAPI::CopyCommandList::SharedPtr RenderContext::CreateCopyCommandList(const U8String& name) const
        {
            ASSERT(inited_);

            auto& resource = GAPI::CopyCommandList::Create(name, GPIObjectsDeleter<GAPI::CopyCommandList>());
            submission_->GetIMultiThreadDevice().lock()->InitCommandList(*resource.get());

            return resource;
        }

        GAPI::ComputeCommandList::SharedPtr RenderContext::CreateComputeCommandList(const U8String& name) const
        {
            ASSERT(inited_);

            auto& resource = GAPI::ComputeCommandList::Create(name, GPIObjectsDeleter<GAPI::ComputeCommandList>());
            submission_->GetIMultiThreadDevice().lock()->InitCommandList(*resource.get());

            return resource;
        }

        GAPI::GraphicsCommandList::SharedPtr RenderContext::CreateGraphicsCommandList(const U8String& name) const
        {
            ASSERT(inited_);

            auto& resource = GAPI::GraphicsCommandList::Create(name, GPIObjectsDeleter<GAPI::GraphicsCommandList>());
            submission_->GetIMultiThreadDevice().lock()->InitCommandList(*resource.get());

            return resource;
        }

        GAPI::CommandQueue::SharedPtr RenderContext::CreteCommandQueue(GAPI::CommandQueueType type, const U8String& name) const
        {
            ASSERT(inited_)

            auto& resource = GAPI::CommandQueue::Create(type, name, GPIObjectsDeleter<GAPI::CommandQueue>());
            submission_->GetIMultiThreadDevice().lock()->InitCommandQueue(*resource.get());

            return resource;
        }

        GAPI::Fence::SharedPtr RenderContext::CreateFence(const U8String& name) const
        {
            ASSERT(inited_);

            auto& resource = GAPI::Fence::Create(name, GPIObjectsDeleter<GAPI::Fence>());
            submission_->GetIMultiThreadDevice().lock()->InitFence(*resource.get());

            return resource;
        }

        GAPI::Texture::SharedPtr RenderContext::CreateTexture(const GAPI::TextureDescription& desc, GAPI::GpuResourceBindFlags bindFlags, const std::shared_ptr<IntermediateMemory>& textureData, const U8String& name) const
        {
            ASSERT(inited_);

            auto& resource = GAPI::Texture::Create(desc, bindFlags, name, GPIObjectsDeleter<GAPI::Texture>());
            submission_->GetIMultiThreadDevice().lock()->InitTexture(*resource.get(), textureData);

            return resource;
        }

        GAPI::Texture::SharedPtr RenderContext::CreateSwapChainBackBuffer(const std::shared_ptr<GAPI::SwapChain>& swapchain, uint32_t backBufferIndex, const GAPI::TextureDescription& desc, GAPI::GpuResourceBindFlags bindFlags, const U8String& name) const
        {
            ASSERT(inited_);
            ASSERT(swapchain);

            auto& resource = GAPI::Texture::Create(desc, bindFlags, name, GPIObjectsDeleter<GAPI::Texture>());
            swapchain->InitBackBufferTexture(backBufferIndex, resource);

            return resource;
        }

        GAPI::ShaderResourceView::SharedPtr RenderContext::CreateShaderResourceView(
            const std::shared_ptr<GAPI::GpuResource>& gpuResource,
            const GAPI::GpuResourceViewDescription& desc) const
        {
            ASSERT(inited_);

            auto& resource = GAPI::ShaderResourceView::Create(gpuResource, desc, GPIObjectsDeleter<GAPI::ShaderResourceView>());
            submission_->GetIMultiThreadDevice().lock()->InitGpuResourceView(*resource.get());

            return resource;
        }

        GAPI::DepthStencilView::SharedPtr RenderContext::CreateDepthStencilView(
            const GAPI::Texture::SharedPtr& texture,
            const GAPI::GpuResourceViewDescription& desc) const
        {
            ASSERT(inited_);

            auto& resource = GAPI::DepthStencilView::Create(texture, desc, GPIObjectsDeleter<GAPI::DepthStencilView>());
            submission_->GetIMultiThreadDevice().lock()->InitGpuResourceView(*resource.get());

            return resource;
        }

        GAPI::RenderTargetView::SharedPtr RenderContext::CreateRenderTargetView(
            const GAPI::Texture::SharedPtr& texture,
            const GAPI::GpuResourceViewDescription& desc) const
        {
            ASSERT(inited_);

            auto& resource = GAPI::RenderTargetView::Create(texture, desc, GPIObjectsDeleter<GAPI::RenderTargetView>());
            submission_->GetIMultiThreadDevice().lock()->InitGpuResourceView(*resource.get());

            return resource;
        }

        GAPI::UnorderedAccessView::SharedPtr RenderContext::CreateUnorderedAccessView(
            const std::shared_ptr<GAPI::GpuResource>& gpuResource,
            const GAPI::GpuResourceViewDescription& desc) const
        {
            ASSERT(inited_);

            auto& resource = GAPI::UnorderedAccessView::Create(gpuResource, desc, GPIObjectsDeleter<GAPI::UnorderedAccessView>());
            submission_->GetIMultiThreadDevice().lock()->InitGpuResourceView(*resource.get());

            return resource;
        }

        GAPI::SwapChain::SharedPtr RenderContext::CreateSwapchain(const GAPI::SwapChainDescription& description, const U8String& name) const
        {
            ASSERT(inited_);

            auto& resource = GAPI::SwapChain::Create(description, name, GPIObjectsDeleter<GAPI::SwapChain>());
            submission_->GetIMultiThreadDevice().lock()->InitSwapChain(*resource.get());

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