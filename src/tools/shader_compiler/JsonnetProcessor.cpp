#include "JsonnetProcessor.hpp"

#include "SubprocessRunner.hpp"

#include <filesystem>
#include <iostream>

namespace RR
{
    JsonnetProcessor::JsonnetProcessor() { }
    JsonnetProcessor::~JsonnetProcessor() { }

    Common::RResult JsonnetProcessor::evaluateFile(const std::string& file, const std::vector<std::string>& includePathes, nlohmann::json& outputJson)
    {
        if (!std::filesystem::exists(file))
        {
            std::cerr << "File not found: " << file << std::endl;
            return Common::RResult::NotFound;
        }

        std::vector<const char*> args = {"jsonnet", file.c_str()};

        if (!includePathes.empty())
        {
            args.push_back("-J");
            for (auto& includePath : includePathes)
                args.push_back(includePath.c_str());
        }

        args.push_back(nullptr);

        SubprocessResult processResult;
        RR_RETURN_ON_FAIL(SubprocessRunner::Run(args, processResult));

        if (processResult.exitCode != 0)
        {
            std::cerr << "Process executed with error: " << processResult.exitCode << std::endl;
            std::cerr << processResult.output << std::endl;
            return Common::RResult::Fail;
        }

        try
        {
            outputJson = nlohmann::json::parse(processResult.output);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Jsonnet file " << file << " parse with error: " << e.what() << std::endl;
            return Common::RResult::Fail;
        }

        return Common::RResult::Ok;
    }
}