#pragma once

#include "gapi/PipelineState.hpp"
#include "gapi/Shader.hpp"

#include "absl/container/flat_hash_map.h"

namespace RR::Common
{
    enum class RResult : int32_t;
    namespace Hashing::Default
    {
        using HashType = uint32_t;
    }
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
        eastl::array<uint32_t, eastl::to_underlying(GAPI::ShaderType::Count)> shaderIndexes;
    };

    struct EffectDesc
    {
        const char* name;
        eastl::vector<PassDesc> passes;
    };

    struct ShaderDesc
    {
        const char* name;
        GAPI::ShaderType type;
        std::byte* data;
        size_t size;
    };

    class EffectLibrary
    {
    public:
        EffectLibrary() = default;
        ~EffectLibrary() = default;

        Common::RResult Load(std::string_view path);
        bool GetEffectDesc(Common::Hashing::Default::HashType hash, EffectDesc& effectDesc) const;

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
        absl::flat_hash_map<Common::Hashing::Default::HashType, uint32_t> effectsMap;
        eastl::vector<EffectDesc> effects;
        eastl::vector<PassDesc> passes;
        eastl::vector<eastl::unique_ptr<std::byte[]>> shadersData;
    };
}