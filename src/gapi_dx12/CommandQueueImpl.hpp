#pragma once

#include "gapi/CommandQueue.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class CommandListImpl;

            class CommandQueueImpl final : public CommandQueueInterface
            {
            public:
                CommandQueueImpl() = delete;
                CommandQueueImpl(CommandQueueType type);

                Result Init(const ComSharedPtr<ID3D12Device>& device, const U8String& name);

                Result Submit(const std::shared_ptr<CommandList>& commandList) override;
                Result Signal(const std::shared_ptr<Fence>& fence, uint64_t value) override;

                Result Wait(const ComSharedPtr<ID3D12Fence>& fence, uint64_t value);

            private:
                CommandQueueType type_;
                ComSharedPtr<ID3D12CommandQueue> D3DCommandQueue_ = nullptr;
            };
        };
    }
}
