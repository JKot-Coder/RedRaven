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

        struct IdComponents
        {
            uint32_t entityIndex : IndexBits;
            uint32_t generation : GenerationBits;
        };

        union
        {
            IdComponents components;
            uint32_t rawId;
        };

        uint32_t GetEntityIndex() const { return components.entityIndex; }
        uint32_t GetGeneration() const { return components.generation; }
        uint32_t Int() const { return InvalidRawId; }

        EntityId() : rawId(InvalidRawId) { }
        EntityId(uint32_t index, uint32_t generation)
        {
            components.entityIndex = index;
            components.generation = generation;
        }
        operator bool() const { return rawId != InvalidRawId; }
    };
}