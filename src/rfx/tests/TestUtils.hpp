#pragma once 

#include <filesystem>

namespace RR::Rfx::Tests
{
    enum class TestType : uint32_t
    {
        CommandLine,
        LexerTest
    };

    void runTestOnFile(const std::filesystem::path& testFile, const std::filesystem::path& testDirectory, TestType type);
    void runTestsInDirectory(const std::filesystem::path& directory, TestType type );
}