#pragma once

namespace RR::Common
{
    class ChunkAllocator final : public Common::NonCopyable
    {
    public:
        struct Chunk
        {
            std::unique_ptr<std::byte[]> buffer;
            std::size_t size;
            std::size_t allocated;

            Chunk(std::size_t size) : buffer(new std::byte[size]), size(size), allocated(0) { ASSERT(IsAlignedTo(buffer.get(), alignof(std::max_align_t))); }
            Chunk(Chunk&& other) noexcept : buffer(std::move(other.buffer)), size(other.size) { }
            Chunk(const Chunk&) = delete;
            Chunk& operator=(const Chunk&) = delete;
        };

    private:
        static constexpr inline size_t MAX_CHUNK_SIZE = 1 << 28;

        std::vector<Chunk> chunks;

        Chunk& getCurrentChunk() { return chunks.back(); }

        void addNewChunk(size_t size)
        {
            ASSERT(size);
            ASSERT(size <= MAX_CHUNK_SIZE);
            ASSERT(IsPowerOfTwo(size));

            chunks.emplace_back(size);
        };

    public:
        explicit ChunkAllocator(std::size_t initialChunkSize)
        {
            addNewChunk(initialChunkSize);
        }

        void reset()
        {
            size_t totalCapasity = 0;

            for (const auto& chunk : chunks)
                totalCapasity += chunk.size;

            totalCapasity = RoundUpToPowerOfTwo(totalCapasity);

            if (chunks.size() == 1 && totalCapasity == chunks.front().size)
            {
                chunks.front().allocated = 0;
                return;
            }

            chunks.clear();
            addNewChunk(totalCapasity);
        }

        void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t))
        {
            ASSERT(size < MAX_CHUNK_SIZE);

            if(size == 0)
                return nullptr;

            Chunk& chunk = getCurrentChunk();
            std::size_t alignedOffset = AlignTo(chunk.allocated, alignment);

            if UNLIKELY (alignedOffset + size > chunk.size)
            {
                size_t nextSize = chunk.size << 1;
                nextSize = std::max(nextSize, RoundUpToPowerOfTwo(size));
                nextSize = std::min(nextSize, MAX_CHUNK_SIZE);
                chunks.emplace_back(nextSize);
                return allocate(size, alignment);
            }
            std::byte* ptr = chunk.buffer.get() + alignedOffset;
            chunk.allocated = alignedOffset + size;
            return ptr;
        }

        char* allocateString(std::string_view str)
        {
             char* ptr = static_cast<char*>(allocate(str.size() + 1, 1));
             std::memcpy(ptr, str.data(), str.size());
             ptr[str.size()] = '\0';
             return ptr;
        }

        template <typename T>
        T* allocateArray(size_t count)
        {
            return static_cast<T*>(allocate(count * sizeof(T), alignof(T)));
        }

        template <typename T, typename... Args>
        T* create(Args&&... args)
        {
            void* memory = allocate(sizeof(T), alignof(T));
            return new (memory) T(std::forward<Args>(args)...);
        }

        auto begin() { return chunks.begin(); }
        auto end() { return chunks.end(); }
        auto size() const { return chunks.size(); }
    };
}