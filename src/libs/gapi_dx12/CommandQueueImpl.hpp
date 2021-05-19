#pragma once

#include "gapi/CommandQueue.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class CommandListImpl;
            class FenceImpl;

            class CommandQueueImpl final : public ICommandQueue
            {
            public:
                CommandQueueImpl() = delete;
                CommandQueueImpl(const CommandQueueImpl& other) : 
                    type_(other.type_), 
                    D3DCommandQueue_(other.D3DCommandQueue_), 
                    fence_(other.fence_) {};
                CommandQueueImpl(CommandQueueType type) : type_(type) {};
                ~CommandQueueImpl();

                void ImmediateD3DObjectRelease();

                void Init(const U8String& name);
                void Submit(const std::shared_ptr<CommandList>& commandList) override;

                // Todo private?
                void Signal(const ComSharedPtr<ID3D12Fence>& fence, uint64_t value);
                void Wait(const ComSharedPtr<ID3D12Fence>& fence, uint64_t value);

                void WaitForGpu() override;

                const ComSharedPtr<ID3D12CommandQueue>& GetD3DObject() const { return D3DCommandQueue_; }

            private:
                CommandQueueType type_;
                ComSharedPtr<ID3D12CommandQueue> D3DCommandQueue_ = nullptr;
                std::shared_ptr<FenceImpl> fence_;
            };
        };
    }
}