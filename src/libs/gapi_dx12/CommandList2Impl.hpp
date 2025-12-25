#pragma once

#include "gapi/CommandList2.hpp"

#include "eastl/queue.h"

#include "gapi_dx12/FenceImpl.hpp"

namespace RR::GAPI::DX12
{
    class CommandList2Impl final : public ICommandList
    {
    private:
        class CommandAllocatorsPool final
        {
        public:
            ~CommandAllocatorsPool();

            void Init(D3D12_COMMAND_LIST_TYPE type, const std::string& name);

            ComSharedPtr<ID3D12CommandAllocator> GetNextAllocator();
            FenceImpl& GetSubmissionFence() const { return *fence_; }

        private:
            using AllocatorFecnceValuePair = std::pair<ComSharedPtr<ID3D12CommandAllocator>, uint64_t>;
            ComSharedPtr<ID3D12CommandAllocator> createAllocator() const;

        private:
            D3D12_COMMAND_LIST_TYPE type_;
            eastl::unique_ptr<FenceImpl> fence_;
            eastl::queue<AllocatorFecnceValuePair> allocators_;
        };

    private:
        // One frame executing on GPU
        static constexpr int InitialAllocatorsCount = MAX_GPU_FRAMES_BUFFERED + 1;
        static constexpr int MaxAllocatorsCount = 16;

    public:
        void Init(const CommandList2& commandList);
        void Compile(CommandList2& commandList);

    private:
        CommandAllocatorsPool allocatorsPool;
    };
}