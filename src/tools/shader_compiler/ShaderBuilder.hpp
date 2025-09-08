#pragma once

#include "common/Singleton.hpp"
#include "common/Result.hpp"

#include "gapi/PipelineState.hpp"
#include "gapi/Shader.hpp"

#include "effect_library/EffectFormat.hpp"

#include "nlohmann/json_fwd.hpp"
#include <common/ChunkAllocator.hpp>

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
        Common::RResult compileEffect(const LibraryBuildDesc& desc, const std::string& sourceFile);
        void evaluateRenderStateDesc(nlohmann::json& effect, GAPI::RasterizerDesc& rasterizerDesc, GAPI::DepthStencilDesc& depthStencilDesc, GAPI::BlendDesc& blendDesc);

        Common::RResult saveLibrary(const LibraryBuildDesc& desc);

        uint32_t pushString(const std::string& str);
        uint32_t pushShader(ShaderResult&& shader);

    private:
        eastl::vector<EffectLibrary::Asset::EffectDesc> effects;
        eastl::vector<ShaderResult> shaders;
        Common::ChunkAllocator stringAllocator;
        uint32_t stringsCount = 0;
    };
}