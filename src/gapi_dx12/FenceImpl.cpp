#include "FenceImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            GAPIStatus FenceImpl::Init(ID3D12Device* device, uint64_t initialValue, const U8String& name)
            {
                ASSERT(device)
                ASSERT(_fence.get() == nullptr)

                GAPIStatus result = GAPIStatus::OK;

                if (GAPIStatusU::Failure(result = GAPIStatus(device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.put())))))
                {
                    LOG_ERROR("Failure create CreateFence with HRESULT of 0x%08X", result);
                    return result;
                }

                D3DUtils::SetAPIName(_fence.get(), name);

                _cpu_value = initialValue;
                return result;
            }

            GAPIStatus FenceImpl::Signal(ID3D12CommandQueue* commandQueue, uint64_t value)
            {
                ASSERT(_fence.get())

                GAPIStatus result = GAPIStatus(commandQueue->Signal(_fence.get(), value));

                if (GAPIStatusU::Failure(result))
                {
                    LOG_ERROR("Failure signal fence with HRESULT of 0x%08X", result);
                    return result;
                }

                _cpu_value = value;
                return result;
            }

            GAPIStatus FenceImpl::SetEventOnCompletion(uint64_t value, HANDLE event) const
            {
                ASSERT(_fence.get())

                GAPIStatus result = GAPIStatus(_fence->SetEventOnCompletion(value, event));

                if (GAPIStatusU::Failure(result))
                    LOG_ERROR("Failure SetEventOnCompletion fence with HRESULT of 0x%08X", result);

                return result;
            }

            uint64_t FenceImpl::GetGpuValue() const
            {
                ASSERT(_fence.get())
                return _fence->GetCompletedValue();
            }
        }       
    }
}