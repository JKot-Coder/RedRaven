#pragma once

#include "slang-com-ptr.h"

namespace RR  {

    namespace Common
    {
        enum class RResult : int32_t;
    }

    namespace GAPI
    {
        enum class ShaderType : uint8_t;
    }

    struct ShaderCompileDesc
    {
        std::vector<std::string> modules;
        std::vector<std::string> includePathes;
        std::vector<std::string> entryPoints;
    };

    struct ShaderResult
    {
        ~ShaderResult() {
            source.setNull();
            // Kill session last
            session.setNull();
        }

        std::string name;
        GAPI::ShaderType type;
        Slang::ComPtr<slang::IBlob> source;
        Slang::ComPtr<slang::ISession> session; // Sources alive until session alive.
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