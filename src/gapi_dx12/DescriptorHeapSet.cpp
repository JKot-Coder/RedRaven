#include "DescriptorHeapSet.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {

            Result DescriptorHeapSet::Init(const ComSharedPtr<ID3D12Device>& device)
            {
                rtvDescriptorHeap_ = std::make_shared<DescriptorHeap>();

                DescriptorHeap::DescriptorHeapDesc desc;
                desc.numDescriptors_ = 1000;
                desc.name = "CpuRtv";
                desc.type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

                D3DCall(rtvDescriptorHeap_->Init(device, desc));

                return Result::Ok;
            }

        }
    }
}