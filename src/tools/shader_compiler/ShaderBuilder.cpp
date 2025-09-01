#include "ShaderBuilder.hpp"

#include <iostream>

#include "JsonnetProcessor.hpp"

namespace RR
{
    ShaderBuilder::ShaderBuilder()
    {
    }

    ShaderBuilder::~ShaderBuilder()
    {
    }

    Common::RResult ShaderBuilder::compileEffect(const std::string& sourceFile)
    {
        std::cout << "Compile compile effect: " << sourceFile << std::endl;

        JsonnetProcessor jsonnetProcessor;
        nlohmann::json outputJson;
        auto result = jsonnetProcessor.evaluateFile(sourceFile, outputJson);
        if (result != Common::RResult::Ok)
        {
            std::cerr << "Failed to read effect file: " << sourceFile << std::endl;
            return Common::RResult::Fail;
        }

        return Common::RResult::Ok;
    }

    Common::RResult ShaderBuilder::BuildLibrary(const LibraryBuildDesc& desc)
    {
        std::cout << "Build shader library: " << desc.inputFile << " -> " << desc.outputFile << std::endl;

        JsonnetProcessor jsonnetProcessor;
        nlohmann::json outputJson;
        auto result = jsonnetProcessor.evaluateFile(desc.inputFile, outputJson);
        if (result != Common::RResult::Ok)
        {
            std::cerr << "Failed to evaluate build list file: " << desc.inputFile << std::endl;
            return Common::RResult::Fail;
        }

        auto sources = outputJson["Sources"];
        if(sources.empty())
        {
            std::cerr << "No sources found in build list file: " << desc.inputFile << std::endl;
            return Common::RResult::Fail;
        }

        for (auto& source : sources)
        {
            if(compileEffect(source.get<std::string>()) != Common::RResult::Ok)
            {
                std::cerr << "Failed to compile shader: " << source << std::endl;
                return Common::RResult::Fail;
            }
        }

        return Common::RResult::Ok;
    }
}