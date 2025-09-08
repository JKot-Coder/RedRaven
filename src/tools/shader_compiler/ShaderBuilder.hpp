#pragma once

#include "common/Singleton.hpp"
#include "common/Result.hpp"

#include "gapi/PipelineState.hpp"
#include "gapi/Shader.hpp"

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

    #pragma pack(push, 1)
    struct Header
    {
        static constexpr uint32_t MAGIC = 0x4C584652;
        static constexpr uint32_t VERSION = 1;
        uint32_t magic;
        uint32_t version;
        uint32_t stringSectionSize;
        uint32_t stringsCount;
        uint32_t effectsSectionSize;
        uint32_t effectsCount;
        uint32_t shadersSectionSize;
        uint32_t shadersCount;
    };

    struct EffectDesc
    {
        uint32_t nameIndex;
        uint32_t passCount;
    };

    static constexpr inline uint32_t INVALID_SHADER_INDEX = static_cast<uint32_t>(-1);
    struct PassDesc
    {
        uint32_t nameIndex;
        GAPI::BlendDesc blendDesc;
        GAPI::RasterizerDesc rasterizerDesc;
        GAPI::DepthStencilDesc depthStencilDesc;
        eastl::array<uint32_t, eastl::to_underlying(GAPI::ShaderType::Count)> shaderIndexes;
    };
    #pragma pack(pop)

    class ShaderBuilder : public Common::Singleton<ShaderBuilder>
    {
    public:
        ShaderBuilder();
        ~ShaderBuilder();

        Common::RResult BuildLibrary(const LibraryBuildDesc& desc);

    private:
        struct Effects
        {
            EffectDesc effectDesc;
            std::vector<PassDesc> passes;
        };

    private:
        Common::RResult compileEffect(const LibraryBuildDesc& desc, const std::string& sourceFile);
        void evaluateRenderStateDesc(nlohmann::json& effect, GAPI::RasterizerDesc& rasterizerDesc, GAPI::DepthStencilDesc& depthStencilDesc, GAPI::BlendDesc& blendDesc);

        Common::RResult saveLibrary(const LibraryBuildDesc& desc);

        uint32_t pushString(const std::string& str);
        uint32_t pushShader(ShaderResult&& shader);

    private:
        eastl::vector<Effects> effects;
        eastl::vector<ShaderResult> shaders;
        Common::ChunkAllocator stringAllocator;
        uint32_t stringsCount = 0;
    };
}