#pragma once

#include <deque>
#include <limits>

#include "gapi/GpuResourceViews.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            class DescriptorHeap : public std::enable_shared_from_this<DescriptorHeap>
            {
            public:
                using SharedPtr = std::shared_ptr<DescriptorHeap>;
                using SharedConstPtr = std::shared_ptr<const DescriptorHeap>;

                struct Allocation;
                struct DescriptorHeapDesc;

                DescriptorHeap() = default;
                ~DescriptorHeap();

                void Init(const DescriptorHeapDesc& desc);

                void Allocate(Allocation& allocation)
                {
                    ASSERT(d3d12Heap_);

                    if (allocated_ >= numDescriptors_)
                        LOG_FATAL("Not enough memory in descriptorHeap: {}", name_);

                    ASSERT(!freeChunks_.empty())

                    Chunk* currentChunk = freeChunks_.front();

                    const auto indexInChunk = currentChunk->Alloc();
                    const auto indexInHeap = indexInChunk + currentChunk->Offset;

                    // Chunk is exhausted
                    if (currentChunk->GetNumAvailable() == 0)
                        freeChunks_.pop_front();

                    allocation = Allocation(shared_from_this(), indexInHeap, getCpuHandle(indexInHeap), getGpuHandle(indexInHeap));

                    allocated_++;
                }

                void Free(uint32_t index)
                {
                    ASSERT(d3d12Heap_);

                    const auto chunkIndex = index / Chunk::SIZE;
                    ASSERT(chunkIndex < chunks_.size());

                    auto& chunk = chunks_[chunkIndex];
                    chunk->Free(index);

                    // Chunk was exhausted
                    if (chunk->GetNumAvailable() == 1)
                        freeChunks_.push_back(chunk.get());

                    allocated_--;
                }

            public:
                struct DescriptorHeapDesc
                {
                    U8String name;
                    uint32_t numDescriptors_;
                    D3D12_DESCRIPTOR_HEAP_TYPE type;
                    D3D12_DESCRIPTOR_HEAP_FLAGS flags;
                };

                struct Allocation final : public IGpuResourceView
                {
                    Allocation() = default;
                    Allocation(Allocation&& donor) noexcept = delete;
                    ~Allocation() { release(); }

                    bool operator==(const Allocation& alloc) const
                    {
                        return (cpuHandle_.ptr == alloc.cpuHandle_.ptr) && (heap_ == alloc.heap_) && (indexInHeap_ == alloc.indexInHeap_);
                    }

                    bool operator!=(const Allocation& alloc) const { return !(*this == alloc); }

                    Allocation& operator=(Allocation&& alloc) noexcept
                    {
                        static_assert(sizeof(Allocation) == 48);

                        std::swap(heap_, alloc.heap_);
                        std::swap(indexInHeap_, alloc.indexInHeap_);
                        std::swap(cpuHandle_, alloc.cpuHandle_);
                        std::swap(gpuHandle_, alloc.gpuHandle_);

                        alloc.release();

                        return *this;
                    }

                    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const
                    {
                        ASSERT(heap_)
                        return cpuHandle_;
                    }

                    CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const
                    {
                        ASSERT(heap_)
                        return gpuHandle_;
                    }

                private:
                    friend DescriptorHeap;

                    Allocation(const DescriptorHeap::SharedPtr& heap, uint32_t indexInHeap, CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle)
                        : heap_(heap),
                          indexInHeap_(indexInHeap),
                          cpuHandle_(cpuHandle),
                          gpuHandle_(gpuHandle)
                    {
                        ASSERT(heap_)
                    }

                    void release()
                    {
                        if (heap_)
                            heap_->Free(indexInHeap_);

                        heap_ = nullptr;
                        indexInHeap_ = 0;
                        cpuHandle_ = CD3DX12_DEFAULT();
                        gpuHandle_ = CD3DX12_DEFAULT();
                    }

                private:
                    DescriptorHeap::SharedPtr heap_ = nullptr;
                    uint32_t indexInHeap_ = 0;
                    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle_ = CD3DX12_DEFAULT();
                    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle_ = CD3DX12_DEFAULT();
                };

            private:
                struct Chunk
                {
                    static constexpr uint32_t SIZE = std::numeric_limits<uint8_t>::max() + 1; // 256
                    uint32_t Offset;

                    Chunk(uint32_t offset) : Offset(offset)
                    {
                        for (int i = 0; i < SIZE; i++)
                            indices_[i] = i;
                    }

                    inline uint32_t Alloc()
                    {
                        ASSERT(cursor_ < SIZE)
                        return static_cast<uint32_t>(indices_[cursor_++]);
                    }

                    inline void Free(uint32_t index)
                    {
                        ASSERT(index < Chunk::SIZE)
                        ASSERT(index >= Offset && index < Offset + Chunk::SIZE)
                        const uint8_t indexInChunk = static_cast<uint8_t>(index - Offset);

                        indices_[--cursor_] = indexInChunk;
                    }

                    inline uint32_t GetNumAvailable() const
                    {
                        return SIZE - cursor_;
                    }

                private:
                    std::array<uint8_t, SIZE> indices_;
                    uint32_t cursor_ = 0;
                };

                CD3DX12_CPU_DESCRIPTOR_HANDLE getCpuHandle(uint32_t index) const
                {
                    ASSERT(d3d12Heap_)
                    return CD3DX12_CPU_DESCRIPTOR_HANDLE(d3d12Heap_->GetCPUDescriptorHandleForHeapStart(), index, descriptorSize_);
                }

                CD3DX12_GPU_DESCRIPTOR_HANDLE getGpuHandle(uint32_t index) const
                {
                    ASSERT(d3d12Heap_)
                    return CD3DX12_GPU_DESCRIPTOR_HANDLE(d3d12Heap_->GetGPUDescriptorHandleForHeapStart(), index, descriptorSize_);
                }

            private:
                U8String name_;

                uint32_t numDescriptors_ = 0;
                uint32_t descriptorSize_ = 0;
                uint32_t allocated_ = 0;

                std::vector<std::unique_ptr<Chunk>> chunks_;
                std::deque<Chunk*> freeChunks_;

                ComSharedPtr<ID3D12DescriptorHeap> d3d12Heap_;
            };
        }
    }
}