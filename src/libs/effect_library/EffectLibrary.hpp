#pragma once

#include "gapi/PipelineState.hpp"
#include "gapi/Shader.hpp"
#include "gapi/BindingLayout.hpp"
#include "effect_library/EffectFormat.hpp" // For Asset enums (VarType, VarKind)

#include "absl/container/flat_hash_map.h"
#include "common/hashing/Hash.hpp"

namespace RR::Common
{
    enum class RResult : int32_t;
}

namespace RR::Render
{
    struct EffectDesc;
}

namespace RR::EffectLibrary
{

    enum class FieldKind : uint8_t
    {
        Array,
        Struct,
        Basic,
    };

    struct FieldReflection
    {
        const char* name;
        Asset::FieldType type;
        FieldKind kind;

        uint32_t structIndex;
        uint32_t arraySize;

        uint32_t offset;
        uint32_t size;
    };

    struct ResourceReflection
    {
        const char* name;

        GAPI::BindingType type;
        GAPI::ShaderStageMask stageMask;

        uint32_t binding;
        uint32_t set;
        uint32_t count;

        uint32_t textureMetaIndex;

        eastl::span<FieldReflection> variables;

        ResourceReflection* child;
        ResourceReflection* next;
    };

    struct ReflectionData
    {
        std::vector<GAPI::BindingLayoutTextureMeta> textureMetas;
        std::vector<ResourceReflection> resources;
        std::vector<FieldReflection> fields;
        ResourceReflection* rootBlock;
    };

    struct PassDesc
    {
        const char* name;
        GAPI::RasterizerDesc rasterizerDesc;
        GAPI::DepthStencilDesc depthStencilDesc;
        GAPI::BlendDesc blendDesc;
        eastl::array<uint32_t, eastl::to_underlying(GAPI::ShaderStage::Count)> shaderIndexes;
        ReflectionData reflection;
    };

    struct EffectDesc
    {
        const char* name;
        eastl::vector<PassDesc> passes;
    };

    struct ShaderDesc
    {
        const char* name;
        GAPI::ShaderStage stage;
        std::byte* data;
        size_t size;
    };

    class EffectLibrary
    {
    public:
        EffectLibrary() = default;
        ~EffectLibrary() = default;

        Common::RResult Load(std::string_view path);
        bool GetEffectDesc(HashType hash, EffectDesc& effectDesc) const;

        size_t GetShaderCount() const { return shaders.size(); }
        const ShaderDesc& GetShader(size_t index) const
        {
            ASSERT(index < shaders.size());
            return shaders[index];
        }

    private:

        const char* getString(uint32_t index) const
        {
            ASSERT(index < strings.size());
            return strings[index];
        }

    private:
        bool loaded = false;
        eastl::unique_ptr<std::byte[]> stringsData;
        eastl::vector<const char*> strings;
        eastl::vector<ShaderDesc> shaders;
        // todo trivial hash
        absl::flat_hash_map<HashType, uint32_t> effectsMap;
        eastl::vector<EffectDesc> effects;
        eastl::vector<PassDesc> passes;
        eastl::vector<eastl::unique_ptr<std::byte[]>> shadersData;
    };
}
