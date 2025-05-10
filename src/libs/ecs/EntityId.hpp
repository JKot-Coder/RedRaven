#pragma once

namespace RR::Ecs
{
    struct EntityId final
    {
        static const uint32_t IndexBits = 24;
        static const uint32_t GenerationBits = 8;
        static_assert(IndexBits + GenerationBits == sizeof(uint32_t) * 8);

        static const uint32_t MaxEntities = (1 << IndexBits) - 1;
        static const uint32_t MaxGenerations = (1 << GenerationBits) - 1;
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

        constexpr EntityId() : rawId(InvalidRawId) { }
        constexpr EntityId(uint32_t id) : rawId(id) { }
        EntityId(uint32_t index, uint32_t generation)
        {
            ASSERT(index < MaxEntities);
            fields.index = index;
            fields.generation = generation;
        }

        uint32_t GetGeneration() const { return fields.generation; }
        uint32_t GetIndex() const { return fields.index; }
        uint32_t GetRawId() const { return rawId; }
        operator bool() const { return rawId != InvalidRawId; }
    };
}