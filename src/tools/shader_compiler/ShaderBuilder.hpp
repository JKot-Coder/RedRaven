#pragma once

#include "common/Singleton.hpp"
#include "common/Result.hpp"

#include "gapi/PipelineState.hpp"
#include "nlohmann/json_fwd.hpp"

namespace RR
{
    struct LibraryBuildDesc
    {
        std::string inputFile;
        std::string outputFile;
        std::vector<std::string> includePathes;
    };

    struct EffectDesc
    {
        GAPI::BlendDesc blendDesc;
        GAPI::RasterizerDesc rasterizerDesc;
        GAPI::DepthStencilDesc depthStencilDesc;
    };

    class ShaderBuilder : public Common::Singleton<ShaderBuilder>
    {
    public:
        ShaderBuilder();
        ~ShaderBuilder();

        Common::RResult BuildLibrary(const LibraryBuildDesc& desc);

    private:
        Common::RResult compileEffect(const LibraryBuildDesc& desc, const std::string& sourceFile);
        void evaluateEffectDesc(nlohmann::json& effect, EffectDesc& effectDesc);
    };
}