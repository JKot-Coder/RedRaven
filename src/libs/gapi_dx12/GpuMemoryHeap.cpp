#include "GpuMemoryHeap.hpp"

#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/ResourceImpl.hpp"

#include "common/Math.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            GpuMemoryHeap::Page::~Page()
            {
            }

            void GpuMemoryHeap::Init(const U8String& name)
            {
                name_ = name;
                getNextPageForAllocation(0, currentPage_);
            }

            void GpuMemoryHeap::Allocate(Allocation& allocation, size_t size, size_t alignment)
            {
                ASSERT(currentPage_);
                ASSERT(size > 0);

                auto pageOffset = AlignTo(currentPage_->offset, alignment);
                if (pageOffset + size > currentPage_->size)
                {
                    pageOffset = 0;

                    usedPages_.push_back(std::move(currentPage_));
                    getNextPageForAllocation(size, currentPage_);
                }

                allocation.offset = pageOffset;
                allocation.fenceValue = 0;
                allocation.resource = currentPage_->resource->GetD3DObject();
                currentPage_->offset = pageOffset + size;
            }

            void GpuMemoryHeap::getNextPageForAllocation(size_t allocSize, std::unique_ptr<Page>& page)
            {
                ASSERT(!page);

                if (freePages_.size() > 0 && freePages_.front()->size > allocSize)
                {
                    page = std::move(freePages_.front());
                    freePages_.pop();
                }
                else
                {
                    const auto pageSize = Max(allocSize, defaultPageSize_);
                    ASSERT(pageSize <= std::numeric_limits<uint32_t>::max());

                    const auto& description = BufferDescription::Create(static_cast<uint32_t>(pageSize));
                    auto& resource = std::make_unique<ResourceImpl>();
                    resource->Init(description, GpuResourceBindFlags::None, fmt::sprintf("%s::%u", name_, pageIndex));

                    page = std::make_unique<Page>(pageSize, std::move(resource));

                    pageIndex++;
                }
            }
        }
    }
}