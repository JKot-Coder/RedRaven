#include "GpuMemoryHeap.hpp"

#include "gapi_dx12/DeviceContext.hpp"

#include "common/Math.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            Result GpuMemoryHeap::Init()
            {
            }

            Result GpuMemoryHeap::Allocate(Allocation& allocation, size_t size, size_t alignment)
            {
                ASSERT(currentPage_);

                auto pageOffset = AlignTo(currentPage_->offset, alignment);
                if (pageOffset + size > currentPage_->size)
                {
                    pageOffset = 0;
                    getNextPageForAlloc(size);
                }

                allocation.offset = pageOffset;
                allocation.fenceValue = 0;
                allocation.resource = currentPage_->resource;
                currentPage_->offset = pageOffset + size;

                return Result::Ok;
            }

            Result GpuMemoryHeap::getNextPageForAlloc(size_t allocSize)
            {
                usedPages_.push_back(std::move(currentPage_));

                if (freePages_.size() > 0 && freePages_.front()->size > allocSize)
                {
                    currentPage_ = std::move(freePages_.front());
                    freePages_.pop();
                }
                else
                {
                    const auto pageSize = Max(allocSize, defaultPageSize_);

                    D3DCallMsg(
                        deviceContext.GetDevice()->CreateCommittedResource(
                            &DefaultHeapProps,
                            D3D12_HEAP_FLAG_NONE,
                            &desc,
                            D3D12_RESOURCE_STATE_COMMON,
                            pOptimizedClearValue,
                            IID_PPV_ARGS(D3DResource_.put())),
                        "GpuMemoryHeap::CreateCommittedResource");

                    currentPage_ = std::make_unique<Page>(pageSize);
                }
            }

        }
    }
}