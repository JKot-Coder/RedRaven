#pragma once

namespace OpenDemo
{
    namespace Render
    {

        //#define CACHE_LINE_ALIGN

        class LinearAllocator
        {
        private:
            static constexpr inline size_t MAX_PAGE_SIZE = 1 << 28;

            struct Page final
            {
                Page() = delete;
                ~Page() = default;

                Page(const Page&) = delete;
                Page& operator=(const Page&) = delete;

                Page(size_t size)
                    : size_(size)
                {
                    ASSERT(size);
                    buffer_.reset(new uint8_t[size]);
                }

                void* Allocate(size_t size, size_t aligment,
#ifdef CACHE_LINE_ALIGN
                    size_t cacheLineSize);
#else
                    size_t);
#endif

                inline void Reset()
                {
                    allocated_ = 0;
                }

                inline size_t GetRemained() const
                {
                    return size_ - allocated_;
                }

                inline size_t GetAllocated() const
                {
                    return size_;
                }

                inline size_t GetSize() const
                {
                    return size_;
                }

            private:
                size_t size_;
                size_t allocated_ = 0;
                std::unique_ptr<uint8_t> buffer_;
            };

        public:
            LinearAllocator() = delete;
            virtual ~LinearAllocator() = default;

            LinearAllocator(const LinearAllocator&) = delete;
            LinearAllocator& operator=(const LinearAllocator&) = delete;

            LinearAllocator(
                size_t baseSize,
#ifdef CACHE_LINE_ALIGN
                size_t cacheLineSize = 128)
                : cacheLineSize_(cacheLineSize)
#else
                size_t = 0)
#endif
            {
                addNewPage(baseSize);
            }

            void* Allocate(const size_t size);

            template <typename T, typename... Args>
            T* Create(Args&&... args)
            {
                static_assert(std::is_trivially_copyable<T>::value);

                const size_t size = sizeof(T);
                void* ptr = Allocate(size);
                return new (ptr) T(std::forward<Args>(args)...);
            }

            void Reset();

            inline void Free()
            {
                const size_t baseCapasity = pages_.front()->GetSize();
                pages_.clear();
            }

        private:
            inline Page* currentPage()
            {
                return pages_.back().get();
            }

            inline void addNewPage(size_t size);

            std::vector<std::unique_ptr<Page>> pages_;
            static constexpr inline size_t alignment_ = 16;
#ifdef CACHE_LINE_ALIGN
            size_t cacheLineSize_;
#else
            static constexpr inline size_t cacheLineSize_ = 0;
#endif
        };
    }
}

#ifdef USE_INLINE
#include "LinearAllocator.inl"
#endif