#include "DeviceContext.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Device.hpp"
#include "gapi/Fence.hpp"
#include "gapi/Framebuffer.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "gapi_dx12/Device.hpp"

#include "render/Submission.hpp"

namespace RR
{
    namespace Render
    {
        DeviceContext::DeviceContext()
            : submission_(new Submission())
        {
        }

        DeviceContext::~DeviceContext() { }

        void DeviceContext::Init()
        {
            ASSERT(!inited_);

            auto debugMode = GAPI::Device::DebugMode::Retail;
#ifdef DEBUG
            // Force debug
            debugMode = GAPI::Device::DebugMode::Debug;
#endif

            GAPI::Device::Description description(GpuFramesBuffered, debugMode);

            const auto& device = GAPI::Device::Create(description, "Primary");
            submission_->Start(device);

            // Init Device
            submission_->ExecuteAwait([](GAPI::Device& device)
                                      {
                                          if (!GAPI::DX12::InitDevice(device))
                                              LOG_FATAL("Can't init device");
                                      });

            inited_ = true;

            fence_ = CreateFence("Frame sync fence");
        }

        void DeviceContext::Terminate()
        {
            ASSERT(inited_);

            //Release resources before termination;
            fence_.reset();

            submission_->Terminate();
            inited_ = false;
        }

        void DeviceContext::Submit(const std::shared_ptr<GAPI::CommandQueue>& commandQueue, const std::shared_ptr<GAPI::CommandList>& commandList)
        {
            ASSERT(inited_);

            submission_->Submit(commandQueue, commandList);
        }

        void DeviceContext::Present(const std::shared_ptr<GAPI::SwapChain>& swapChain)
        {
            ASSERT(inited_);

            submission_->ExecuteAsync([swapChain](GAPI::Device& device)
                                      { device.Present(swapChain); });
        }

        void DeviceContext::WaitForGpu(const std::shared_ptr<GAPI::CommandQueue>& commandQueue)
        {
            ASSERT(inited_);

            submission_->ExecuteAwait([commandQueue](GAPI::Device&)
                                      { commandQueue->WaitForGpu(); });
        }

        void DeviceContext::MoveToNextFrame(const std::shared_ptr<GAPI::CommandQueue>& commandQueue)
        {
            ASSERT(inited_);

            static uint32_t submissionFrame = 0;

            // TODO this is not valid. We should sync all command list with primary command list.
            submission_->ExecuteAsync([fence = fence_, commandQueue](GAPI::Device& device)
                                      {
                                          submissionFrame++;

                                          // Schedule a Signal command in the queue.
                                          fence->Signal(commandQueue);

                                          if (fence->GetCpuValue() >= GpuFramesBuffered)
                                          {
                                              uint64_t syncFenceValue = fence->GetCpuValue() - GpuFramesBuffered;

                                              // We shoud had at least one completed frame in ringbuffer.
                                              syncFenceValue++;

                                              // Throttle cpu if gpu behind
                                              fence->SyncCPU(syncFenceValue, GAPI::INFINITY_WAIT);
                                          }

                                          device.MoveToNextFrame(submissionFrame);
                                      });

            // Todo throttle main thread?
        }

        void DeviceContext::ExecuteAsync(const Submission::CallbackFunction&& function)
        {
            ASSERT(inited_);

            submission_->ExecuteAsync(std::move(function));
        }

        void DeviceContext::ExecuteAwait(const Submission::CallbackFunction&& function)
        {
            ASSERT(inited_);

            submission_->ExecuteAwait(std::move(function));
        }

        void DeviceContext::ResetSwapChain(const std::shared_ptr<GAPI::SwapChain>& swapchain, GAPI::SwapChainDescription& description)
        {
            ASSERT(inited_);

            submission_->ExecuteAwait([&swapchain, &description](GAPI::Device&)
                                      { swapchain->Reset(description); });
        }

        GAPI::CopyCommandList::SharedPtr DeviceContext::CreateCopyCommandList(const U8String& name) const
        {
            ASSERT(inited_);

            auto resource = GAPI::CopyCommandList::Create(name);
            submission_->GetIMultiThreadDevice().lock()->InitCommandList(*resource.get());

            return resource;
        }

        GAPI::ComputeCommandList::SharedPtr DeviceContext::CreateComputeCommandList(const U8String& name) const
        {
            ASSERT(inited_);

            auto resource = GAPI::ComputeCommandList::Create(name);
            submission_->GetIMultiThreadDevice().lock()->InitCommandList(*resource.get());

            return resource;
        }

        GAPI::GraphicsCommandList::SharedPtr DeviceContext::CreateGraphicsCommandList(const U8String& name) const
        {
            ASSERT(inited_);

            auto resource = GAPI::GraphicsCommandList::Create(name);
            submission_->GetIMultiThreadDevice().lock()->InitCommandList(*resource.get());

            return resource;
        }

        GAPI::CommandQueue::SharedPtr DeviceContext::CreteCommandQueue(GAPI::CommandQueueType type, const U8String& name) const
        {
            ASSERT(inited_)

            auto resource = GAPI::CommandQueue::Create(type, name);
            submission_->GetIMultiThreadDevice().lock()->InitCommandQueue(*resource.get());

            return resource;
        }

        GAPI::Framebuffer::SharedPtr DeviceContext::CreateFramebuffer(const GAPI::FramebufferDesc& desc) const
        {
            ASSERT(inited_);

            auto resource = GAPI::Framebuffer::Create(desc);
            submission_->GetIMultiThreadDevice().lock()->InitFramebuffer(*resource.get());

            return resource;
        }

        GAPI::Fence::SharedPtr DeviceContext::CreateFence(const U8String& name) const
        {
            ASSERT(inited_);

            auto resource = GAPI::Fence::Create(name);
            submission_->GetIMultiThreadDevice().lock()->InitFence(*resource.get());

            return resource;
        }

        GAPI::GpuResourceFootprint DeviceContext::GetResourceFootprint(const GAPI::GpuResourceDescription& desc) const
        {
            ASSERT(inited_);
            return submission_->GetIMultiThreadDevice().lock()->GetResourceFootprint(desc);
        }

        GAPI::Buffer::SharedPtr DeviceContext::CreateBuffer(
            const GAPI::GpuResourceDescription& desc,
            IDataBuffer::SharedPtr initialData,
            const U8String& name) const
        {
            ASSERT(inited_);

            auto resource = GAPI::Buffer::Create(desc, initialData, name);
            submission_->GetIMultiThreadDevice().lock()->InitBuffer(resource);

            return resource;
        }

        GAPI::Texture::SharedPtr DeviceContext::CreateTexture(
            const GAPI::GpuResourceDescription& desc,
            IDataBuffer::SharedPtr initialData,
            const U8String& name) const
        {
            ASSERT(inited_);

            auto resource = GAPI::Texture::Create(desc, initialData, name);
            submission_->GetIMultiThreadDevice().lock()->InitTexture(resource);

            return resource;
        }

        GAPI::Texture::SharedPtr DeviceContext::CreateSwapChainBackBuffer(
            const std::shared_ptr<GAPI::SwapChain>& swapchain,
            uint32_t backBufferIndex,
            const GAPI::GpuResourceDescription& desc,
            const U8String& name) const
        {
            ASSERT(inited_);
            ASSERT(swapchain);
            ASSERT(desc.dimension == GAPI::GpuResourceDimension::Texture2D);
            ASSERT(desc.usage == GAPI::GpuResourceUsage::Default);
            ASSERT(desc.GetNumSubresources() == 1);

            auto resource = GAPI::Texture::Create(desc, nullptr, name);
            swapchain->InitBackBufferTexture(backBufferIndex, resource);

            return resource;
        }

        GAPI::ShaderResourceView::SharedPtr DeviceContext::CreateShaderResourceView(
            const std::shared_ptr<GAPI::GpuResource>& gpuResource,
            const GAPI::GpuResourceViewDescription& desc) const
        {
            ASSERT(inited_);

            auto resource = GAPI::ShaderResourceView::Create(gpuResource, desc);
            submission_->GetIMultiThreadDevice().lock()->InitGpuResourceView(*resource.get());

            return resource;
        }

        GAPI::DepthStencilView::SharedPtr DeviceContext::CreateDepthStencilView(
            const GAPI::Texture::SharedPtr& texture,
            const GAPI::GpuResourceViewDescription& desc) const
        {
            ASSERT(inited_);

            auto resource = GAPI::DepthStencilView::Create(texture, desc);
            submission_->GetIMultiThreadDevice().lock()->InitGpuResourceView(*resource.get());

            return resource;
        }

        GAPI::RenderTargetView::SharedPtr DeviceContext::CreateRenderTargetView(
            const GAPI::Texture::SharedPtr& texture,
            const GAPI::GpuResourceViewDescription& desc) const
        {
            ASSERT(inited_);

            auto resource = GAPI::RenderTargetView::Create(texture, desc);
            submission_->GetIMultiThreadDevice().lock()->InitGpuResourceView(*resource.get());

            return resource;
        }

        GAPI::UnorderedAccessView::SharedPtr DeviceContext::CreateUnorderedAccessView(
            const std::shared_ptr<GAPI::GpuResource>& gpuResource,
            const GAPI::GpuResourceViewDescription& desc) const
        {
            ASSERT(inited_);

            auto resource = GAPI::UnorderedAccessView::Create(gpuResource, desc);
            submission_->GetIMultiThreadDevice().lock()->InitGpuResourceView(*resource.get());

            return resource;
        }

        GAPI::SwapChain::SharedPtr DeviceContext::CreateSwapchain(const GAPI::SwapChainDescription& description, const U8String& name) const
        {
            ASSERT(inited_);

            auto resource = GAPI::SwapChain::Create(description, name);
            submission_->GetIMultiThreadDevice().lock()->InitSwapChain(*resource.get());

            return resource;
        }
    }
}