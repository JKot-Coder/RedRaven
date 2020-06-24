#pragma once

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {
                class FenceImpl final
                {
                public:
                    // _commandList->SetName(L"CommandList");
                    FenceImpl() = default;
                    GAPIStatus Init(ID3D12Device* device, uint64_t initialValue);

                    GAPIStatus Signal(ID3D12CommandQueue* commandQueue, uint64_t value) const;
                    GAPIStatus SetEventOnCompletion(uint64_t value, HANDLE event) const;

                    uint64_t GetGpuValue() const;

                private:
                    ComSharedPtr<ID3D12Fence> _fence;
                };

            }
        }
    }
}