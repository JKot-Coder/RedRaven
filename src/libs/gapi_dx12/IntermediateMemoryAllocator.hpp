#pragma once

#include "gapi/MemoryAllocation.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class ResourceImpl;

            class CpuAllocation final : public IMemoryAllocation
            {
            public:
                CpuAllocation(size_t size) { memory_ = operator new(size); };
                ~CpuAllocation() { operator delete(memory_); }

                void* Map() const override { return memory_; }
                void Unmap() const override { }

            private:
                void* memory_;
            };

            class HeapAllocation final : public IMemoryAllocation
            {
            public:
                HeapAllocation(D3D12_HEAP_TYPE heapType, size_t size);
                ~HeapAllocation();

                void* Map() const override;
                void Unmap() const override;

            private:
                mutable bool isMapped_ = false;
                size_t size_;                
                std::shared_ptr<ResourceImpl> resource_;
            };

            class IntermediateMemoryAllocator
            {
            public:
                static std::shared_ptr<IntermediateMemory> const IntermediateMemoryAllocator::AllocateIntermediateTextureData(
                    const TextureDescription& resourceDesc,
                    MemoryAllocationType memoryType,
                    uint32_t firstSubresourceIndex,
                    uint32_t numSubresources);
            };
        }
    }
}