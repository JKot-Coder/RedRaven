#pragma once

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            class CommandListImpl;

            class CommandQueueImpl final
            {
            public:
                CommandQueueImpl() = delete;
                CommandQueueImpl(CommandQueueType type);

                Result Init(const ComSharedPtr<ID3D12Device>& device, const U8String& name);
//                Result Submit(CommandListInterface& CommandContext) override;

            private:
                CommandQueueType type_;
                ComSharedPtr<ID3D12CommandQueue> D3DCommandQueue_ = nullptr;
            };
        };
    }
}
