#include "FenceImpl.hpp"

#include "gapi/CommandQueue.hpp"

#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            void FenceImpl::ReleaseD3DObjects(ResourceReleaseContext& releaseContext)
            {
                releaseContext.DeferredD3DResourceRelease(D3DFence_);
            }

            Result FenceImpl::Init(const ComSharedPtr<ID3D12Device>& device, const U8String& name)
            {
                ASSERT(device);
                ASSERT(!D3DFence_);

                D3DCallMsg(device->CreateFence(cpuValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(D3DFence_.put())), "CreateFence");
                D3DUtils::SetAPIName(D3DFence_.get(), name);

                event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                ASSERT(event_);

                return Result::Ok;
            }

            Result FenceImpl::Signal(const std::shared_ptr<CommandQueue>& queue)
            {
                ASSERT(queue);

                const auto queueImpl = queue->GetPrivateImpl<CommandQueueImpl>();
                ASSERT(queueImpl);

                return Signal(*queueImpl);
            }

            Result FenceImpl::Signal(CommandQueueImpl& queue)
            {
                ASSERT(D3DFence_);

                cpuValue_++;

                D3DCall(queue.Signal(D3DFence_, cpuValue_));

                return Result::Ok;
            }

            Result FenceImpl::SyncCPU(std::optional<uint64_t> value, uint32_t timeout) const
            {
                ASSERT(D3DFence_);

                uint64_t syncVal = value ? value.value() : cpuValue_;
                ASSERT(syncVal <= cpuValue_);

                uint64_t gpuVal = GetGpuValue();
                if (gpuVal < syncVal)
                {
                    D3DCallMsg(D3DFence_->SetEventOnCompletion(syncVal, event_), "SetEventOnCompletion");
                    return Result(WaitForSingleObject(event_, timeout));
                }

                return Result::Ok;
            }

            Result FenceImpl::SyncGPU(const std::shared_ptr<CommandQueue>& queue) const
            {
                ASSERT(D3DFence_);
                ASSERT(queue);
                ASSERT(dynamic_cast<CommandQueueImpl*>(queue->GetPrivateImpl()));

                const auto& queueImpl = static_cast<CommandQueueImpl*>(queue->GetPrivateImpl());
                D3DCall(queueImpl->Wait(D3DFence_, cpuValue_));

                return Result::Ok;
            }
        }
    }
}