#include "FenceImpl.hpp"

#include "gapi/CommandQueue.hpp"

#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            FenceImpl::~FenceImpl()
            {
                if (!event_)
                    return;

                CloseHandle(event_);
            }

            void FenceImpl::Init(const U8String& name)
            {
                ASSERT(!D3DFence_);

                const auto& device = DeviceContext::GetDevice();

                D3DCall(device->CreateFence(cpuValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(D3DFence_.put())));
                D3DUtils::SetAPIName(D3DFence_.get(), name);

                event_ = CreateEvent(nullptr, false, false, nullptr);
                ASSERT(event_);
            }

            uint64_t FenceImpl::Increment()
            {
                ASSERT(D3DFence_);

                cpuValue_++;

                return cpuValue_;
            }

            void FenceImpl::Wait(std::optional<uint64_t> value, uint32_t timeout) const
            {
                ASSERT(D3DFence_);

                uint64_t syncVal = value ? value.value() : cpuValue_;
                ASSERT(syncVal <= cpuValue_);

                uint64_t gpuVal = GetGpuValue();
                if (gpuVal < syncVal)
                {
                    D3DCall(D3DFence_->SetEventOnCompletion(syncVal, event_));
                    D3DCall(WaitForSingleObject(event_, timeout == INFINITY_WAIT ? INFINITE : timeout));
                }
            }
        }
    }
}