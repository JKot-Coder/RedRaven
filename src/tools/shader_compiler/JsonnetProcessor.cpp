#include "JsonnetProcessor.hpp"

#include "common/OnScopeExit.hpp"

#include "subprocess.h"

#include <filesystem>
#include <iostream>

namespace RR
{
    JsonnetProcessor::JsonnetProcessor() { }
    JsonnetProcessor::~JsonnetProcessor() { }

    Common::RResult JsonnetProcessor::evaluateFile(const std::string& file, nlohmann::json& outputJson)
    {
        if (!std::filesystem::exists(file))
        {
            std::cerr << "File not found: " << file << std::endl;
            return Common::RResult::NotFound;
        }

        std::vector<const char*> args = {"jsonnet", file.c_str(), nullptr};

        subprocess_s process;
        auto runCode = subprocess_create(args.data(),
                                         subprocess_option_combined_stdout_stderr | subprocess_option_no_window,
                                         &process);

        ON_SCOPE_EXIT([&process]() {
            UNUSED(subprocess_destroy(&process));
        });

        if (runCode != 0)
        {
            std::cerr << "Process executed with error: " << runCode << std::endl;
            return Common::RResult::Fail;
        }

        char buffer[4096];
        std::string output;
        while (unsigned size = subprocess_read_stdout(&process, buffer, sizeof(buffer)))
            output.append(buffer, size);

        int exitCode;
        subprocess_join(&process, &exitCode);

        if (exitCode != 0)
        {
            std::cerr << "Process executed with error: " << exitCode << std::endl;
            std::cerr << output << std::endl;
            return Common::RResult::Fail;
        }

        try
        {
            outputJson = nlohmann::json::parse(output);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Jsonnet file " << file << " parse with error: " << e.what() << std::endl;
            return Common::RResult::Fail;
        }

        return Common::RResult::Ok;
    }
}