#pragma once

struct subprocess_s;

namespace RR
{
    namespace Common
    {
        enum class RResult : int32_t;
    }

    struct SubprocessResult
    {
        int exitCode = 0;
        std::string output;
    };

    class SubprocessRunner
    {
    public:
        static Common::RResult Run(const std::vector<const char*>& args, SubprocessResult& result);

    private:
        SubprocessRunner() = delete;
    };
}

