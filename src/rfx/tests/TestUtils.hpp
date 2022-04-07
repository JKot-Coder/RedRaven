#pragma once 

#include <filesystem>

namespace RR::Rfx::Tests
{
    void runTestOnFile(const std::filesystem::path& testFile, const std::filesystem::path& testDirectory);
    void runTestsInDirectory(const std::filesystem::path& directory);
}