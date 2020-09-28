#include "FenceImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            Result FenceImpl::Init(ID3D12Device* device, uint64_t initialValue, const U8String& name)
            {
                ASSERT(device)
                ASSERT(_fence.get() == nullptr)

                D3DCallMsg(device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.put())), "CreateFence");
                D3DUtils::SetAPIName(_fence.get(), name);

                _cpu_value = initialValue;
                return Result::OK;
            }

            Result FenceImpl::Signal(ID3D12CommandQueue* commandQueue, uint64_t value)
            {
                ASSERT(_fence.get())

                D3DCallMsg(commandQueue->Signal(_fence.get(), value), "Signal");

                _cpu_value = value;
                return Result::OK;
            }

            Result FenceImpl::SetEventOnCompletion(uint64_t value, HANDLE event) const
            {
                ASSERT(_fence.get())

                D3DCallMsg(_fence->SetEventOnCompletion(value, event), "SetEventOnCompletion");

                return Result::OK;
            }

            uint64_t FenceImpl::GetGpuValue() const
            {
                ASSERT(_fence.get())
                return _fence->GetCompletedValue();
            }
        }
    }
}