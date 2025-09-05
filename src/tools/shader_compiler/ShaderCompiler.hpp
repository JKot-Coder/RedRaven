#pragma once

namespace RR  {

    namespace Common
    {
        enum class RResult : int32_t;
    }

    struct ShaderCompileDesc
    {
        std::vector<std::string> modules;
        std::vector<std::string> includePathes;
        std::vector<std::string> entryPoints;
    };

    class ShaderCompiler
    {
    public:
        ShaderCompiler();
        ~ShaderCompiler();

        Common::RResult CompileShader(const ShaderCompileDesc& desc);
    };

}