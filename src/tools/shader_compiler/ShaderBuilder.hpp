#pragma once

#include "EffectSerializer.hpp"

#include "common/Singleton.hpp"
#include "common/Result.hpp"

#include "gapi/PipelineState.hpp"
#include "gapi/Shader.hpp"

#include "effect_library/EffectFormat.hpp"

#include "nlohmann/json_fwd.hpp"
#include "slang-com-ptr.h"

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
        Common::RResult BuildLibrary(const LibraryBuildDesc& desc);

    private:
        Common::RResult compileFile(const LibraryBuildDesc& desc, const std::string& sourceFile);
        void compileEffect(const std::string& name, nlohmann::json effect, const std::string& sourceFile);
        void evaluateRenderStateDesc(nlohmann::json& effect, GAPI::RasterizerDesc& rasterizerDesc, GAPI::DepthStencilDesc& depthStencilDesc, GAPI::BlendDesc& blendDesc);

        Common::RResult saveLibrary(const LibraryBuildDesc& desc);


    private:
        EffectSerializer effectSerializer;
        Slang::ComPtr<slang::IGlobalSession> globalSession;
    };
}