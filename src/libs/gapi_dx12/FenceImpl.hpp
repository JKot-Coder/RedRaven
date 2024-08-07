#pragma once

#include "gapi/Fence.hpp"

namespace RR
{
    namespace GAPI::DX12
    {
        class CommandQueueImpl;

        class FenceImpl final : public IFence
        {
        public:
            FenceImpl() = default;
            ~FenceImpl();

            void Init(const std::string& name);

            uint64_t Increment()
            {
                ASSERT(D3DFence_);
                return ++cpuValue_;
            }

            void Wait(std::optional<uint64_t> value, uint32_t timeout = INFINITY_WAIT) const override;

            uint64_t GetGpuValue() const override
            {
                ASSERT(D3DFence_);
                return D3DFence_->GetCompletedValue();
            }
            uint64_t GetCpuValue() const override { return cpuValue_; }

            const ComSharedPtr<ID3D12Fence>& GetD3DObject() const { return D3DFence_; }

        private:
            HANDLE event_ = 0;
            ComSharedPtr<ID3D12Fence> D3DFence_ = nullptr;
            uint64_t cpuValue_ = 1;
        };
    }
}