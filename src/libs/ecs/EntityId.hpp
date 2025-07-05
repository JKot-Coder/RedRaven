#pragma once

namespace RR::Ecs
{
    struct EntityId final
    {
    public:
        using IndexType = uint32_t;
        static const uint32_t IndexBits = 24;
        static const uint32_t GenerationBits = 8;
        static_assert(IndexBits + GenerationBits == sizeof(uint32_t) * 8u);

        static const uint32_t MaxEntities = (1u << IndexBits) - 1u;
        static const uint32_t MaxGenerations = (1u << GenerationBits) - 1u;
        static const uint32_t InvalidRawId = 0xFFFFFFFFu;

    public:
        constexpr EntityId() : rawId(InvalidRawId) { }
        constexpr explicit EntityId(uint32_t id) : rawId(id) { }
        EntityId(uint32_t index, uint32_t generation)
        {
            ASSERT(index < MaxEntities);
            fields.index = index;
            fields.generation = generation;
        }

        [[nodiscard]] uint32_t GetGeneration() const { return fields.generation; }
        [[nodiscard]] uint32_t GetIndex() const { return fields.index; }
        [[nodiscard]] uint32_t GetRawId() const { return rawId; }
        [[nodiscard]] explicit operator bool() const { return rawId != InvalidRawId; }

        bool operator==(const EntityId& other) const { return rawId == other.rawId; }
        bool operator!=(const EntityId& other) const { return rawId != other.rawId; }

    private:
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
    };
}