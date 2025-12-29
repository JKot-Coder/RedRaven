#pragma once

#include "common/hashing/Hash.hpp"
#include "gapi/Limits.hpp"

namespace RR::GAPI
{
    struct VertexLayoutBuilder;

    enum class VertexAttributeType : uint16_t
    {
        Float,
        Half,

        Uint8,
        Int8,
        Unorm8,
        Snorm8,

        Uint16,
        Int16,
        Unorm16,
        Snorm16,

        Uint32,
        Int32,
        Unorm32,
        Snorm32,
    };

    struct VertexAttributeDesc
    {
        const char* semanticName;
        uint32_t semanticIndex;
        VertexAttributeType type;
        uint16_t numComponents;
        uint32_t bufferSlot;
    };

    struct VertexLayout
    {
    private:
        eastl::fixed_vector<VertexAttributeDesc, MAX_VERTEX_ATTRIBUTES, false> attributes;
        Common::HashType hash;

        friend struct VertexLayoutBuilder;

    public:
        Common::HashType GetHash() const { return hash; }

        size_t GetAttributeCount() const { return attributes.size(); }
        const VertexAttributeDesc& GetAttribute(size_t index) const { return attributes[index]; }

        static VertexLayoutBuilder Build();
    };

    struct VertexLayoutBuilder
    {
        VertexLayoutBuilder& Add(const char* semanticName, uint32_t semanticIndex, VertexAttributeType type, uint16_t numComponents, uint32_t bufferSlot)
        {
            layout.attributes.emplace_back(VertexAttributeDesc {semanticName, semanticIndex, type, numComponents, bufferSlot});
            hash.Combine(semanticName);
            hash.Combine(semanticIndex);
            hash.Combine(type);
            hash.Combine(numComponents);
            hash.Combine(bufferSlot);
            return *this;
        }

        [[nodiscard]] VertexLayout Commit() {
            layout.hash = hash.GetHash();
            return layout;
        }

        VertexLayout layout;
        Common::HashBuilder<Common::DefaultHasher> hash;
    };

    inline VertexLayoutBuilder VertexLayout::Build() { return VertexLayoutBuilder(); }
}