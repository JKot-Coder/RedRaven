#pragma once

#include "gapi/CommandQueue.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class CommandListImpl;

            class CommandQueueImpl final : public ICommandQueue
            {
            public:
                CommandQueueImpl() = delete;
                CommandQueueImpl(CommandQueueType type) : type_(type) {};

                void ReleaseD3DObjects();

                void Init(const U8String& name);
                void Submit(const std::shared_ptr<CommandList>& commandList) override;

                // Todo private?
                void Signal(const ComSharedPtr<ID3D12Fence>& fence, uint64_t value);
                void Wait(const ComSharedPtr<ID3D12Fence>& fence, uint64_t value);

                const ComSharedPtr<ID3D12CommandQueue>& GetD3DObject() const { return D3DCommandQueue_; }

            private:
                CommandQueueType type_;
                ComSharedPtr<ID3D12CommandQueue> D3DCommandQueue_ = nullptr;
            };
        };
    }
}