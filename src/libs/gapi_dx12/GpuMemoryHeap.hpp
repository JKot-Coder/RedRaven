#pragma once

//TODO remove
#include "gapi/Buffer.hpp"
#include "gapi/MemoryAllocation.hpp"

#include <queue>

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class ResourceImpl;

            class GpuMemoryHeap
            {
            public:
                struct Allocation final : public IMemoryAllocation
                {
                    Allocation(size_t size, size_t offset, const ComSharedPtr<ID3D12Resource>& resource) : size_(size), offset_(offset), resource_(resource) {};
                    ~Allocation();

                    ComSharedPtr<ID3D12Resource> GetD3DResouce() const { return resource_; }
                    size_t GetOffset() const { return offset_; }

                    void* Map() const override;
                    void Unmap() const override;

                private:
                    size_t size_;
                    size_t offset_;
                    uint64_t fenceValue_ = 0;
                    ComSharedPtr<ID3D12Resource> resource_;
                    mutable bool isMapped_ = false;
                };

                struct Page
                {
                    Page() = delete;
                    ~Page();
                    Page(size_t size, std::unique_ptr<ResourceImpl>&& resource) : size(size), resource(std::move(resource)) {};

                    size_t size;
                    size_t offset = 0;
                    std::unique_ptr<ResourceImpl> resource;
                };

            public:
                GpuMemoryHeap(size_t pageSize) : defaultPageSize_(pageSize) {};

                void Init(GpuResourceCpuAccess cpuAcess, const U8String& name);
                GpuMemoryHeap::Allocation* Allocate(size_t size, size_t alignment = 1);

            private:
                void getNextPageForAllocation(size_t allocSize, std::unique_ptr<Page>& page);

            private:
                size_t defaultPageSize_;
                U8String name_ = "";
                GpuResourceCpuAccess cpuAcess_;
                uint32_t pageIndex = 0;
                std::unique_ptr<Page> currentPage_;
                std::queue<std::unique_ptr<Page>> freePages_;
                std::vector<std::unique_ptr<Page>> usedPages_;
            };
        }
    }
}