#pragma once

#include <filesystem>
namespace fs = std::filesystem;

namespace RR
{
    namespace Rfx
    {
        enum class RfxResult : int32_t;

        class IRfxFileSystem
        {
        public:
            virtual fs::file_status GetPathStatus(const fs::path& path) const = 0;
            virtual RfxResult GetPathUniqueIdentity(const fs::path& path, U8String& indetity) const = 0;
            //       virtual RfxResult CalcCombinedPath(SlangPathType fromPathType, const U8String& fromPath, const U8String& path, fs::path& pathOut) = 0;
        };

        struct IRfxMutableFileSystem : public IRfxFileSystem
        {
        };

        class OSFileSystem final : public IRfxFileSystem
        {
        public:
            virtual fs::file_status GetPathStatus(const fs::path& path) const override;

            virtual RfxResult GetPathUniqueIdentity(const fs::path& path, U8String& indetity) const override;
            //        virtual RfxResult CalcCombinedPath(SlangPathType fromPathType, const U8String& fromPath, const U8String& path, fs::path& pathOut) override;
        };
    }
}