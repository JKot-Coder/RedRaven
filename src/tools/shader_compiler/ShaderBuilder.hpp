#pragma once

#include "common/Singleton.hpp"
#include "common/Result.hpp"

#include "gapi/PipelineState.hpp"
#include "gapi/Shader.hpp"

#include "effect_library/EffectFormat.hpp"

#include "nlohmann/json_fwd.hpp"
#include <common/ChunkAllocator.hpp>

#include "slang-com-ptr.h"
#include "absl/container/flat_hash_map.h"

namespace RR
{
    struct ShaderResult;

    struct LibraryBuildDesc
    {
        std::string inputFile;
        std::string outputFile;
        std::vector<std::string> includePathes;
    };

    class ShaderBuilder : public Common::Singleton<ShaderBuilder>
    {
    public:
        ShaderBuilder();
        ~ShaderBuilder();

        Common::RResult BuildLibrary(const LibraryBuildDesc& desc);
    private:
        Common::RResult compileFile(const LibraryBuildDesc& desc, const std::string& sourceFile);
        void compileEffect(const std::string& name, nlohmann::json effect, const std::string& sourceFile);
        void evaluateRenderStateDesc(nlohmann::json& effect, GAPI::RasterizerDesc& rasterizerDesc, GAPI::DepthStencilDesc& depthStencilDesc, GAPI::BlendDesc& blendDesc);

        Common::RResult saveLibrary(const LibraryBuildDesc& desc);

        uint32_t pushString(std::string_view str);
        uint32_t pushShader(ShaderResult&& shader);

    private:
        eastl::vector<EffectLibrary::Asset::EffectDesc> effects;
        eastl::vector<EffectLibrary::Asset::ShaderDesc> shaders;
        eastl::vector<ShaderResult> shaderResults;
        Common::ChunkAllocator stringAllocator;
        uint32_t stringsCount = 0;
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        absl::flat_hash_map<std::string, uint32_t> stringsCache;
    };
}