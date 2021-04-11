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

                ComSharedPtr<ID3D12Resource> GetD3DResouce() const;
                size_t GetOffset() const { return 0; }

            private:
                mutable bool isMapped_ = false;
                size_t size_;
                D3D12_HEAP_TYPE heapType_;
                std::shared_ptr<ResourceImpl> resource_;
            };

            class CpuResourceDataAllocator
            {
            public:
                static std::shared_ptr<CpuResourceData> const CpuResourceDataAllocator::Alloc(
                    const GpuResourceDescription& resourceDesc,
                    MemoryAllocationType memoryType,
                    uint32_t firstSubresourceIndex,
                    uint32_t numSubresources);
            };
        }
    }
}