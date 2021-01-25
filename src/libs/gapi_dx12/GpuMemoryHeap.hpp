#pragma once

#include "gapi/Result.hpp"

#include <queue>

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class GpuMemoryHeap
            {
            public:
                struct Allocation
                {
                    size_t size = 0;
                    uint64_t fenceValue = 0;
                };

                struct Page
                {
                    Page() = delete;
                    Page(size_t size, const ComSharedPtr<ID3D12Resource>& resource) : size(size), resource(resource) {};

                    size_t size;
                    size_t offset = 0;
                    ComSharedPtr<ID3D12Resource> resource;
                };

            public:
                Result Init();
                Result Allocate(Allocation& allocation, size_t size, size_t alignment = 1);

            private:
                Result getNextPageForAlloc(size_t allocSize);

            private:
                size_t defaultPageSize_;
                std::unique_ptr<Page> currentPage_;
                std::queue<std::unique_ptr<Page>> freePages_;
                std::vector<std::unique_ptr<Page>> usedPages_;
            };
        }
    }
}