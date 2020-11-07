#pragma once

#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/FencedRingBuffer.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            class CommandListImpl final
            {
            public:
                CommandListImpl() = delete;
                CommandListImpl(D3D12_COMMAND_LIST_TYPE type)
                    : _type(type)
                {
                }
                Result Init(ID3D12Device* device, const U8String& name);

                ComSharedPtr<ID3D12GraphicsCommandList> GetCommandList()
                {
                    return _commandList;
                };

                Result Submit(ID3D12CommandQueue* queue);

            private:
                D3D12_COMMAND_LIST_TYPE _type;
                ComSharedPtr<ID3D12GraphicsCommandList> _commandList;
                std::unique_ptr<FencedFrameRingBuffer<ID3D12CommandAllocator*>> _allocatorsRB;
            };
        }
    }
}