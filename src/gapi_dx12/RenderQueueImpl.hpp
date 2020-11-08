#pragma once

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            class CommandListImpl;

            class RenderQueueImpl final
            {
            public:
                RenderQueueImpl() = delete;
                RenderQueueImpl(D3D12_COMMAND_LIST_TYPE type);

                Result Init(ID3D12Device* device, const U8String& name);
//                Result Submit(RenderCommandContextInterface& RenderCommandContext) override;

            private:
                D3D12_COMMAND_LIST_TYPE type_;
                ComSharedPtr<ID3D12CommandQueue> D3DCommandQueue_ = nullptr;
            };
        };
    }
}
