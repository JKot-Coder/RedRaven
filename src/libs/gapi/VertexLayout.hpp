#pragma once

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
        eastl::fixed_vector<VertexAttributeDesc, 8> attributes;

    public:
        static VertexLayoutBuilder Build();
    };

    struct VertexLayoutBuilder
    {
        VertexLayoutBuilder& Add(const char* semanticName, uint32_t semanticIndex, VertexAttributeType type, uint16_t numComponents, uint32_t bufferSlot)
        {
            layout.attributes.emplace_back(VertexAttributeDesc {semanticName, semanticIndex, type, numComponents, bufferSlot});
            return *this;
        }

        [[nodiscard]] VertexLayout Commit() { return layout; }

        VertexLayout layout;
    };

    inline VertexLayoutBuilder VertexLayout::Build() { return VertexLayoutBuilder(); }
}