#include "FenceImpl.hpp"

#include "gapi/CommandQueue.hpp"

#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            void FenceImpl::ReleaseD3DObjects()
            {
                DeviceContext::GetResourceReleaseContext()->DeferredD3DResourceRelease(D3DFence_);
            }

            void FenceImpl::Init(const U8String& name)
            {
                ASSERT(!D3DFence_);

                const auto& device = DeviceContext::GetDevice();

                D3DCall(device->CreateFence(cpuValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(D3DFence_.put())));
                D3DUtils::SetAPIName(D3DFence_.get(), name);

                event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                ASSERT(event_);
            }

            void FenceImpl::Signal(const std::shared_ptr<CommandQueue>& queue)
            {
                ASSERT(queue);

                const auto queueImpl = queue->GetPrivateImpl<CommandQueueImpl>();
                ASSERT(queueImpl);

                return Signal(*queueImpl);
            }

            void FenceImpl::Signal(CommandQueueImpl& queue)
            {
                ASSERT(D3DFence_);

                cpuValue_++;

                queue.Signal(D3DFence_, cpuValue_);
            }

            void FenceImpl::SyncCPU(std::optional<uint64_t> value, uint32_t timeout) const
            {
                ASSERT(D3DFence_);

                uint64_t syncVal = value ? value.value() : cpuValue_;
                ASSERT(syncVal <= cpuValue_);

                uint64_t gpuVal = GetGpuValue();
                if (gpuVal < syncVal)
                {
                    D3DCall(D3DFence_->SetEventOnCompletion(syncVal, event_));
                    D3DCall(WaitForSingleObject(event_, timeout));
                }
            }

            void FenceImpl::SyncGPU(const std::shared_ptr<CommandQueue>& queue) const
            {
                ASSERT(D3DFence_);
                ASSERT(queue);
                ASSERT(dynamic_cast<CommandQueueImpl*>(queue->GetPrivateImpl()));

                const auto& queueImpl = static_cast<CommandQueueImpl*>(queue->GetPrivateImpl());
                queueImpl->Wait(D3DFence_, cpuValue_);
            }
        }
    }
}