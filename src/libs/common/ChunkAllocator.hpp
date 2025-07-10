#pragma once

namespace RR::Common
{
    class ChunkAllocator final : public Common::NonCopyable
    {
    private:
        static constexpr inline size_t MAX_CHUNK_SIZE = 1 << 28;

        struct Chunk
        {
            std::unique_ptr<std::byte[]> buffer;
            std::size_t size;

            Chunk(std::size_t size) : buffer(new std::byte[size]), size(size) { }
            Chunk(Chunk&& other) noexcept : buffer(std::move(other.buffer)), size(other.size) { }
            Chunk(const Chunk&) = delete;
            Chunk& operator=(const Chunk&) = delete;
        };

        std::size_t offset;
        std::vector<Chunk> chunks;

        const Chunk& getCurrentChunk() { return chunks.back(); }

        void addNewChunk(size_t size)
        {
            ASSERT(size);
            ASSERT(size <= MAX_CHUNK_SIZE);
            ASSERT(IsPowerOfTwo(size));

            chunks.emplace_back(size);
            offset = 0;
        };

    public:
        explicit ChunkAllocator(std::size_t initialChunkSize)
        {
            addNewChunk(initialChunkSize);
        }

        void reset()
        {
            size_t totalCapasity = 0;

            for (const auto& page : chunks)
                totalCapasity += page.size;

            totalCapasity = RoundUpToPowerOfTwo(totalCapasity);

            if (chunks.size() == 1 && totalCapasity == chunks.front().size)
            {
                offset = 0;
                return;
            }

            chunks.clear();
            addNewChunk(totalCapasity);
        }

        void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t))
        {
            ASSERT(size < MAX_CHUNK_SIZE);

            const Chunk& chunk = getCurrentChunk();
            std::size_t alignedOffset = AlignTo(offset, alignment);

            if UNLIKELY (alignedOffset + size > chunk.size)
            {
                size_t nextSize = chunk.size << 1;
                nextSize = std::max(nextSize, RoundUpToPowerOfTwo(size));
                nextSize = std::min(nextSize, MAX_CHUNK_SIZE);
                chunks.emplace_back(nextSize);
                return allocate(size, alignment);
            }
            void* ptr = chunk.buffer.get() + alignedOffset;
            offset = alignedOffset + size;
            return ptr;
        }

        template <typename T, typename... Args>
        T* create(Args&&... args)
        {
            void* memory = allocate(sizeof(T), alignof(T));
            return new (memory) T(std::forward<Args>(args)...);
        }
    };
}