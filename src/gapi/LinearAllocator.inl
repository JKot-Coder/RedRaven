#include "common/Math.hpp"

namespace OpenDemo
{
    namespace Render
    {
        INLINE void* LinearAllocator::Page::Allocate(size_t size, size_t aligment,
#ifdef CACHE_LINE_ALIGN
            size_t cacheLineSize)
#else
            size_t)
#endif
        {
            ASSERT(size)

            if (GetRemained() < size)
                return nullptr;

#ifdef CACHE_LINE_ALIGN
            if (cacheLineSize > 0)
            {
                const size_t cacheLineOffset = allocated_ % cacheLineSize;
                if (size < cacheLineSize && cacheLineOffset + size > cacheLineSize)
                {
                    const size_t pad = cacheLineSize - cacheLineOffset;
                    allocated_ += pad;
                }

                // Not enough space afer aligment
                if (GetRemained() < size)
                    return nullptr;
            }
#endif

            const auto head = &buffer_.get()[allocated_];
            allocated_ += size;

            // Align offset
            allocated_ = Common::PointerMath::AlignTo(allocated_, aligment);

#ifdef ENABLE_ASSERTS
            // Check pointer is aligned
            ASSERT(Common::PointerMath::IsAlignedTo(head, aligment));
#endif

            return head;
        }

        INLINE void* LinearAllocator::Allocate(const std::size_t size)
        {
            ASSERT(size)
            ASSERT(size <= MAX_PAGE_SIZE)

            for (;;)
            {
                const auto page = currentPage();

                const auto result = page->Allocate(size, alignment_, cacheLineSize_);

                // No enough space. Allocate new page
                if (result == nullptr)
                {
                    size_t nextSize = page->GetSize() << 1;
                    nextSize = std::min(nextSize, MAX_PAGE_SIZE);
                    addNewPage(nextSize);

                    continue;
                }

                return result;
            }
        }

        INLINE void LinearAllocator::Reset()
        {
            size_t totalCapasity = 0;

            for (const auto& page : pages_)
                totalCapasity += page->GetSize();

            Free();
            addNewPage(Common::RoundUpToPowerOfTwo(totalCapasity));
        }

        INLINE void LinearAllocator::addNewPage(size_t size)
        {
            ASSERT(size)
            ASSERT(size <= MAX_PAGE_SIZE)
            ASSERT(Common::IsPowerOfTwo(size))

            pages_.push_back(std::move(std::make_unique<Page>(size)));
        }

    }
}
