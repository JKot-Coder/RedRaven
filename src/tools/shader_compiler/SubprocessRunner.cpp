#include "SubprocessRunner.hpp"

#include "common/OnScopeExit.hpp"
#include "common/Result.hpp"
#include "subprocess.h"

#include "common/io/FileSystem.hpp"

namespace RR
{
    Common::RResult SubprocessRunner::Run(const std::vector<const char*>& args, SubprocessResult& result)
    {
        ASSERT(!args.empty() && args.back() == nullptr);

        if (args.empty() || args.back() != nullptr)
            return Common::RResult::InvalidArgument;

        subprocess_s process;
        auto runCode = subprocess_create(args.data(),
                                         subprocess_option_combined_stdout_stderr |
                                             subprocess_option_no_window |
                                             subprocess_option_search_user_path,
                                         &process);

        if (runCode != 0)
            return Common::RResult::Fail;

        ON_SCOPE_EXIT([&process]() {
            subprocess_destroy(&process);
        });

        char buffer[4096];
        std::string output;
        while (unsigned size = subprocess_read_stdout(&process, buffer, sizeof(buffer)))
            output.append(buffer, size);

        int exitCode;
        if (subprocess_join(&process, &exitCode) != 0)
            return Common::RResult::Fail;

        result.exitCode = exitCode;
        result.output = std::move(output);

        return Common::RResult::Ok;
    }
}

