#pragma once

#include "gapi/MemoryAllocation.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
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