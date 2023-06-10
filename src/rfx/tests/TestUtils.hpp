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
        enum class TestType : uint32_t
        {
            CommandLine,
            LexerTest
        };


        void runTestOnFile(const std::filesystem::path& testFile, const std::filesystem::path& testDirectory, TestType type);
        void runTestsInDirectory(const std::filesystem::path& directory);
        void runTestsInDirectory(const std::filesystem::path& directory, TestType type);
        void verify(const std::filesystem::path& testFile, Common::RResult result);

        template <typename T>
        void runTestsInDirectory2(const std::filesystem::path& directory, T testCase)
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
            {
                if (entry.path().extension() != ".rfx")
                    continue;

                std::error_code ec;
                auto flieName = fs::relative(entry, directory, ec);
                ASSERT(!(bool)ec);

                Common::RResult result = testCase(flieName.u8string(), directory.u8string());
                verify(flieName, result);
            }
        }
    }
}