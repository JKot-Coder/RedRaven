#include "DescriptorHeap.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            DescriptorHeap::~DescriptorHeap()
            {
                // No any leaks
                ASSERT(allocated_ == 0);
            }

            Result DescriptorHeap::Init(const ComSharedPtr<ID3D12Device>& device, const DescriptorHeapDesc& desc)
            {
                ASSERT(device)
                ASSERT(!d3d12Heap_)
                ASSERT(desc.numDescriptors_ > 0)

                name_ = desc.name;
                numDescriptors_ = desc.numDescriptors_;

                descriptorSize_ = device->GetDescriptorHandleIncrementSize(desc.type);
                uint32_t chunkCount = (numDescriptors_ + Chunk::SIZE - 1) / Chunk::SIZE;

                D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
                rtvDescriptorHeapDesc.NumDescriptors = chunkCount * Chunk::SIZE;
                rtvDescriptorHeapDesc.Type = desc.type;
                rtvDescriptorHeapDesc.Flags = desc.flags;

                D3DCallMsg(device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(d3d12Heap_.put())), "CreateDescriptorHeap");

                D3DUtils::SetAPIName(d3d12Heap_.get(), name_);

                for (uint32_t i = 0; i < chunkCount; i++)
                {
                    uint32_t offset = i * Chunk::SIZE;
                    chunks_.emplace_back(std::make_unique<Chunk>(offset));
                    freeChunks_.emplace_back(chunks_[i].get());
                }

                return Result::Ok;
            }
        }
    }
}