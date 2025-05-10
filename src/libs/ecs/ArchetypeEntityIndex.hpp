#pragma once

namespace RR::Ecs
{
    struct ArchetypeEntityIndex final
    {
        static const uint32_t IndexBits = 18;
        static const uint32_t ChunkBits = 14;

        static const uint32_t MaxEntitiesCount = 1U << IndexBits;
        static const uint32_t MaxChunksCount = 1U << ChunkBits;
        static const uint32_t ChunkMask = MaxChunksCount - 1;
        static const uint32_t InvalidRawIndex = 0xFFFFFFFF;

        constexpr ArchetypeEntityIndex() : rawIndex(InvalidRawIndex) { };
        constexpr ArchetypeEntityIndex(uint32_t index) : rawIndex(index) { }
        static constexpr ArchetypeEntityIndex Invalid() { return ArchetypeEntityIndex {}; };
        ArchetypeEntityIndex(uint32_t indexInChunk, uint32_t chunkIndex)
        {
            ASSERT(indexInChunk < MaxEntitiesCount);
            ASSERT(chunkIndex < MaxChunksCount);
            fields.indexInChunk = indexInChunk;
            fields.chunk = chunkIndex;
        }

        uint32_t GetChunkIndex() const { return fields.chunk; }
        uint32_t GetIndexInChunk() const { return fields.indexInChunk; }
        uint32_t GetRaw() const { return rawIndex; }

        explicit operator bool() const { return rawIndex != InvalidRawIndex; }
        bool operator==(const ArchetypeEntityIndex other) const { return rawIndex == other.rawIndex; }
        bool operator!=(const ArchetypeEntityIndex other) const { return rawIndex != other.rawIndex; }

    private:
        struct IndexFields
        {
            uint32_t indexInChunk : IndexBits;
            uint32_t chunk : ChunkBits;
        };
        union
        {
            IndexFields fields;
            uint32_t rawIndex;
        };
    };
    static_assert(sizeof(ArchetypeEntityIndex) == sizeof(uint32_t));

}