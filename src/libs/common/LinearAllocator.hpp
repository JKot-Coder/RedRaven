#pragma once

namespace RR
{
    namespace Common
    {
        // #define CACHE_LINE_ALIGN

        class LinearAllocator : private NonCopyable
        {
        private:
            static constexpr inline size_t MAX_PAGE_SIZE = 1 << 28;

            struct Page final : private NonCopyable
            {
                Page() = delete;
                ~Page() = default;

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

                inline size_t GetRemained() const { return size_ - allocated_; }
                inline size_t GetAllocated() const { return size_; }
                inline size_t GetSize() const { return size_; }

                inline void Reset()
                {
                    allocated_ = 0;
                }

            private:
                size_t size_;
                size_t allocated_ = 0;
                std::unique_ptr<uint8_t> buffer_;
            };

        public:
            LinearAllocator() = delete;
            virtual ~LinearAllocator() = default;

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
                pages_.clear();
            }

        private:
            inline Page* currentPage()
            {
                return pages_.back().get();
            }

            INLINE void addNewPage(size_t size);

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

#if ENABLE_INLINE
#include "LinearAllocator.inl"
#endif