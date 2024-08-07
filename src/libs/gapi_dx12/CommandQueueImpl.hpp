#pragma once

#include "gapi/CommandQueue.hpp"

namespace RR
{
    namespace GAPI::DX12
    {
        class CommandListImpl;
        class FenceImpl;

        class CommandQueueImpl final : public ICommandQueue
        {
        public:
            CommandQueueImpl(const CommandQueueImpl& other) : type_(other.type_),
                                                              D3DCommandQueue_(other.D3DCommandQueue_),
                                                              fence_(other.fence_) {};
            CommandQueueImpl(CommandQueueType type);
            ~CommandQueueImpl();

            std::any GetNativeHandle() const override { return D3DCommandQueue_.get(); }

            void ImmediateD3DObjectRelease();

            void Init(const std::string& name);
            void Submit(const std::shared_ptr<CommandList>& commandList) override;
            void Submit(CommandListImpl& commandList, bool waitForPendingUploads);

            void Signal(const std::shared_ptr<Fence>& fence) override;
            void Signal(const std::shared_ptr<Fence>& fence, uint64_t value) override;
            void Signal(FenceImpl& fence);
            void Signal(FenceImpl& fence, uint64_t value);

            void Wait(const FenceImpl& fence, uint64_t value);
            void WaitForGpu() override;

            const ComSharedPtr<ID3D12CommandQueue>& GetD3DObject() const { return D3DCommandQueue_; }

        private:
            D3D12_COMMAND_LIST_TYPE type_;
            ComSharedPtr<ID3D12CommandQueue> D3DCommandQueue_ = nullptr;
            std::shared_ptr<FenceImpl> fence_;
        };
    };
}