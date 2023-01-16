#pragma once

#include "FileSystem.hpp"

namespace RR::Common::IO
{
    class OSFileSystem final : public IFileSystem
    {
    public:
        virtual fs::file_status GetPathStatus(const fs::path& path) const override;
        virtual RResult GetPathUniqueIdentity(const fs::path& path, U8String& indetity) const override;
        //        virtual RResult CalcCombinedPath(SlangPathType fromPathType, const U8String& fromPath, const U8String& path, fs::path& pathOut) override;
    };
}