#pragma once

#include "slang-com-ptr.h"

namespace RR  {

    namespace Common
    {
        enum class RResult : int32_t;
    }

    struct PassDesc;
    class EffectSerializer;

    struct ShaderCompileDesc
    {
        std::vector<std::string> modules;
        std::vector<std::string> includePathes;
        std::vector<std::string> entryPoints;
        EffectSerializer* effectSerializer = nullptr;
    };

    class ShaderCompiler
    {
    public:
        ShaderCompiler();
        ~ShaderCompiler();

        Common::RResult CompileShader(const Slang::ComPtr<slang::IGlobalSession>& globalSession, const ShaderCompileDesc& desc, RR::PassDesc& passDesc);
    };

}