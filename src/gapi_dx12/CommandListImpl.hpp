#pragma once

#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/FencedRingBuffer.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {

                class CommandListImpl final
                {
                public:
                    // _commandList->SetName(L"CommandList");
                    CommandListImpl() = delete;
                    CommandListImpl(D3D12_COMMAND_LIST_TYPE type)
                        : _type(type)
                    {
                    }
                    GAPIStatus Init(ID3D12Device* device, std::shared_ptr<FenceImpl>& fence);

                    ComSharedPtr<ID3D12GraphicsCommandList> GetCommandList()
                    {
                        return _commandList;
                    };

                    void MoveToNextFrame();

                private:
                    D3D12_COMMAND_LIST_TYPE _type;
                    ComSharedPtr<ID3D12GraphicsCommandList> _commandList;
                    std::unique_ptr<FencedFrameRingBuffer<ComSharedPtr<ID3D12CommandAllocator>>> _allocatorsRB;
                };

            }
        }
    }
}