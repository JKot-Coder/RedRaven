#include "DescriptorHeapSet.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {

            GAPIStatus DescriptorHeapSet::Init(ID3D12Device* device)
            {
                rtvDescriptorHeap_ = std::make_unique<DescriptorHeap>();

                GAPIStatus result;

                DescriptorHeap::DescriptorHeapDesc desc;
                desc.numDescriptors_ = 1000;
                desc.name = "Rtv heap";
                desc.type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

                if (GAPIStatusU::Failure(result = GAPIStatus(rtvDescriptorHeap_->Init(device, desc))))
                {
                    LOG_ERROR("Failure init DescriptorHeap with HRESULT of 0x%08X", result);
                    return result;
                }

                return result;
            }

        }
    }
}