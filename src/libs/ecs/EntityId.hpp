#pragma once

namespace RR::Ecs
{
    struct EntityId
    {
        static const uint32_t IndexBits = 24;
        static const uint32_t GenerationBits = 8;
        static_assert(IndexBits + GenerationBits == sizeof(uint32_t) * 8);

        static const uint32_t MaxEntitiesCount = 1 << IndexBits;
        static const uint32_t MaxGenerationsCount = 1 << GenerationBits;
        static const uint32_t GenerationsMask = MaxGenerationsCount - 1;
        static const uint32_t InvalidRawId = 0xFFFFFFFF;
        struct IdFields
        {
            uint32_t index : IndexBits;
            uint32_t generation : GenerationBits;
        };
        union
        {
            IdFields fields;
            uint32_t rawId;
        };

        EntityId() : rawId(InvalidRawId) { }
        EntityId(uint32_t index, uint32_t generation)
        {
            fields.index = index;
            fields.generation = generation;
        }

        uint32_t GetGeneration() const { return fields.generation; }
        uint32_t GetIndex() const { return fields.index; }
        operator bool() const { return rawId != InvalidRawId; }
    };
}