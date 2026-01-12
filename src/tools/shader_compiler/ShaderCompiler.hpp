#pragma once

#include "slang-com-ptr.h"

namespace RR  {

    namespace Common
    {
        enum class RResult : int32_t;
    }

    namespace GAPI
    {
        enum class ShaderStage : uint8_t;
    }

    class EffectSerializer;

    struct ShaderCompileDesc
    {
        std::vector<std::string> modules;
        std::vector<std::string> includePathes;
        std::vector<std::string> entryPoints;
        EffectSerializer* effectSerializer = nullptr;
    };

    struct ShaderResult
    {
        ~ShaderResult() { }
        std::string name;
        GAPI::ShaderStage stage;
        std::vector<std::byte> source;
    };

    struct CompileResult
    {
        std::vector<ShaderResult> shaders;
    };

    class ShaderCompiler
    {
    public:
        ShaderCompiler();
        ~ShaderCompiler();

        Common::RResult CompileShader(const Slang::ComPtr<slang::IGlobalSession>& globalSession, const ShaderCompileDesc& desc, CompileResult& result);
    };

}