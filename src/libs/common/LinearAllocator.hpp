#pragma once

namespace RR
{
    namespace Common
    {
        template<size_t Alignment = 16>
        class LinearAllocator : private NonCopyable
        {
        protected:
            static constexpr inline size_t MAX_PAGE_SIZE = 1 << 28;

            struct Page final : private NonCopyable
            {
                Page() = delete;
                ~Page() = default;

                Page(size_t size) : size_(size)
                {
                    ASSERT(size);
                    buffer_.reset(new uint8_t[size]);
                }

                void* Allocate(size_t size,
#ifdef CACHE_LINE_ALIGN
                               size_t cacheLineSize);
#else
                               size_t);
#endif
                inline void* GetData() const { return &buffer_.get()[0]; };
                inline size_t GetRemained() const { return size_ - allocated_; }
                inline size_t GetAllocated() const { return allocated_; }
                inline size_t GetSize() const { return size_; }

                inline void Reset()
                {
                    allocated_ = 0;
                }

            private:
                size_t size_;
                size_t allocated_ = 0;
                std::unique_ptr<uint8_t[]> buffer_;
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

            inline void* Allocate(const size_t size);

            template <typename T, typename... Args>
            T* Create(Args&&... args)
            {
                static_assert(std::is_trivially_copyable<T>::value);

                const size_t size = sizeof(T);
                void* ptr = Allocate(size);
                return new (ptr) T(std::forward<Args>(args)...);
            }

            inline void Reset();

            inline void Free()
            {
                pages_.clear();
            }

        protected:
            inline Page* currentPage()
            {
                return pages_.back().get();
            }

            inline void addNewPage(size_t size)
            {
                ASSERT(size);
                ASSERT(size <= MAX_PAGE_SIZE);
                ASSERT(IsPowerOfTwo(size));

                pages_.push_back(std::make_unique<Page>(size));
            };

            std::vector<std::unique_ptr<Page>> pages_;
#ifdef CACHE_LINE_ALIGN
            size_t cacheLineSize_;
#else
            static constexpr inline size_t cacheLineSize_ = 0;
#endif
        };

        template<size_t Alignment>
        void* LinearAllocator<Alignment>::Page::Allocate(size_t size,
#ifdef CACHE_LINE_ALIGN
                                                     size_t cacheLineSize)
#else
                                                     size_t)
#endif
        {
            ASSERT(size);

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
            allocated_ = AlignTo(allocated_, Alignment);

            // Check pointer is aligned
            ASSERT(IsAlignedTo(head, Alignment));

            return head;
        }

        template<size_t Alignment>
        void* LinearAllocator<Alignment>::Allocate(const std::size_t size)
        {
            ASSERT(size);
            ASSERT(size <= MAX_PAGE_SIZE);

            for (;;)
            {
                const auto page = currentPage();

                const auto result = page->Allocate(size, cacheLineSize_);

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

        template<size_t Alignment>
        void LinearAllocator<Alignment>::Reset()
        {
            size_t totalCapasity = 0;

            for (const auto& page : pages_)
                totalCapasity += page->GetSize();

            totalCapasity = RoundUpToPowerOfTwo(totalCapasity);

            if (pages_.size() == 1 && totalCapasity == pages_.front()->GetSize())
            {
                pages_.front()->Reset();
                return;
            }

            Free();
            addNewPage(totalCapasity);
        }
    }
}