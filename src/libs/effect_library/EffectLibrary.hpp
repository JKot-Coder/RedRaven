#pragma once

#include "gapi/PipelineState.hpp"
#include "gapi/Shader.hpp"
#include "gapi/BindingGroupLayout.hpp"
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
    struct PassDesc
    {
        const char* name;
        GAPI::RasterizerDesc rasterizerDesc;
        GAPI::DepthStencilDesc depthStencilDesc;
        GAPI::BlendDesc blendDesc;
        eastl::array<uint32_t, eastl::to_underlying(GAPI::ShaderStage::Count)> shaderIndexes;
        uint32_t rootBindingGroupIndex;
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
        const std::byte* data;
        size_t size;
    };

    struct BindingGroupReflection
    {
        const char* name;
        uint32_t bindingSpace;
    };

    struct ResourceReflection
    {
        Asset::ResourceType type;
        const char* name;
        GAPI::ShaderStageMask usageMask;
        uint32_t binding;
        uint32_t count;
        // SRV and UAV
        GAPI::GpuResourceDimension dimension;
        // SRV only
        GAPI::TextureSampleType sampleType;
        // UAV only
        GAPI::GpuResourceFormat format;
        // CBV only
        uint32_t layoutIndex;
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
        eastl::vector<BindingGroupReflection> bindingGroupReflections;
        eastl::vector<ResourceReflection> resources;
    };
}
