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

                GAPIStatus Init(ID3D12Device* device, const U8String& name);
//                GAPIStatus Submit(RenderContextInterface& renderContext) override;

            private:
                D3D12_COMMAND_LIST_TYPE type_;
                ComSharedPtr<ID3D12CommandQueue> D3DCommandQueue_ = nullptr;
            };
        };
    }
}
