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

            GpuMemoryHeap::Allocation GpuMemoryHeap::Allocate(size_t size, size_t alignment)
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

                Allocation allocation;
                allocation.offset = pageOffset;
                allocation.fenceValue = 0;
                allocation.resource = currentPage_->resource->GetD3DObject();

                D3D12_RANGE readRange = { 0, 0 };
                currentPage_->resource->Map(0, readRange, allocation.mappedData);

                currentPage_->offset = pageOffset + size;

                return allocation;
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

                    const auto& description = BufferDescription::Create(static_cast<uint32_t>(pageSize), BufferDescription::CpuAccess::Write);
                    auto& resource = std::make_unique<ResourceImpl>();
                    resource->Init(description, GpuResourceBindFlags::None, fmt::sprintf("%s::%u", name_, pageIndex));

                    page = std::make_unique<Page>(pageSize, std::move(resource));

                    pageIndex++;
                }
            }
        }
    }
}