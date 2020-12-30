#pragma once

#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/FencedRingBuffer.hpp"

namespace OpenDemo
{
    namespace GAPI
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
                Result Init(const ComSharedPtr<ID3D12Device>& device, const U8String& name);

                const ComSharedPtr<ID3D12GraphicsCommandList>& GetD3DObject() const { return D3DCommandList_; } 

                Result Reset();

                Result Submit(ID3D12CommandQueue* queue);

            private:
                D3D12_COMMAND_LIST_TYPE _type;
                ComSharedPtr<ID3D12GraphicsCommandList> D3DCommandList_;
                std::unique_ptr<FencedFrameRingBuffer<ID3D12CommandAllocator*>> _allocatorsRB;
            };
        }
    }
}