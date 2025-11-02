#include "JsonnetProcessor.hpp"

#include "common/OnScopeExit.hpp"

#include "subprocess.h"

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

        subprocess_s process;
        auto runCode = subprocess_create(args.data(),
                                         subprocess_option_combined_stdout_stderr |
                                             subprocess_option_no_window |
                                             subprocess_option_search_user_path,
                                         &process);

        if (runCode != 0)
        {
            std::cerr << "Process " << args[0] << " executed with error: " << runCode << std::endl;
            return Common::RResult::Fail;
        }

        ON_SCOPE_EXIT([&process]() {
            UNUSED(subprocess_destroy(&process));
        });

        char buffer[4096];
        std::string output;
        while (unsigned size = subprocess_read_stdout(&process, buffer, sizeof(buffer)))
            output.append(buffer, size);

        int exitCode;
        for (int result = subprocess_join(&process, &exitCode); result != 0;) {
            std::cerr << "Process " << args[0] << " failed to join." << result << std::endl;
#ifndef OS_WINDOWS
            std::cerr << "errno: " << errno << " (" << strerror(errno) << ")" << std::endl;
#endif
            return Common::RResult::Fail;
        }

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