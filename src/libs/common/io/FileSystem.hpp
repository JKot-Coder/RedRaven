#pragma once

#include <filesystem>

namespace RR::Common::IO
{
    enum class RResult : int32_t;
    namespace fs = std::filesystem;

    class IFileSystem
    {
    public:
        virtual fs::file_status GetPathStatus(const fs::path& path) const = 0;
        virtual RResult GetPathUniqueIdentity(const fs::path& path, U8String& indetity) const = 0;
        //       virtual RResult CalcCombinedPath(SlangPathType fromPathType, const U8String& fromPath, const U8String& path, fs::path& pathOut) = 0;
    };
}