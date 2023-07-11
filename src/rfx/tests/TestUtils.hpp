#pragma once

#include <filesystem>

namespace RR
{
    namespace Common
    {
        enum class RResult : int32_t;
    }

    namespace Rfx::Tests
    {
        void runTestOnFile(const std::filesystem::path& testFile, const std::filesystem::path& testDirectory);
        void runTestsInDirectory(const std::filesystem::path& directory);
    }
}