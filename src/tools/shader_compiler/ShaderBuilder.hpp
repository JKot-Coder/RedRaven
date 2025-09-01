#pragma once

#include "common/Singleton.hpp"
#include "common/Result.hpp"

namespace RR
{
    struct LibraryBuildDesc
    {
        std::string inputFile;
        std::string outputFile;
    };

    class ShaderBuilder : public Common::Singleton<ShaderBuilder>
    {
    public:
        ShaderBuilder();
        ~ShaderBuilder();

        Common::RResult BuildLibrary(const LibraryBuildDesc& desc);

    private:
        Common::RResult compileEffect(const std::string& sourceFile);
    };
}