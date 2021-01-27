#include "DescriptorHeapSet.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            void DescriptorHeapSet::Init()
            {
                rtvDescriptorHeap_ = std::make_shared<DescriptorHeap>();

                DescriptorHeap::DescriptorHeapDesc desc;
                desc.numDescriptors_ = 1000;
                desc.name = "CpuRtv";
                desc.type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

                rtvDescriptorHeap_->Init(desc);
            }
        }
    }
}