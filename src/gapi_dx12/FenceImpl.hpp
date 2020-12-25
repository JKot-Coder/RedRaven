#pragma once

#include "gapi/Fence.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class FenceImpl final : public FenceInterface
            {
            public:
                FenceImpl() = default;
                Result Init(const ComSharedPtr<ID3D12Device>& device, uint64_t initialValue, const U8String& name);

                Result SyncCPU(std::optional<uint64_t> value, uint32_t timeout) const override;
                Result SyncGPU(const std::shared_ptr<CommandQueue>& queue) const override;

                uint64_t GetGpuValue() const override
                {
                    ASSERT(D3DFence_)
                    return D3DFence_->GetCompletedValue();
                }
                uint64_t GetCpuValue() const override { return cpuValue_; }

                const ComSharedPtr<ID3D12Fence>& GetD3DObject() const { return D3DFence_; }

            private:
                HANDLE event_ = 0;
                ComSharedPtr<ID3D12Fence> D3DFence_ = nullptr;
                uint64_t cpuValue_ = 0;
            };
        }
    }
}