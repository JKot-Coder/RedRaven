#pragma once

#include <filesystem>
namespace fs = std::filesystem;

namespace RR
{
    namespace Common
    {
        enum class RResult : int32_t;
    }

    namespace Rfx
    {
        class IRfxFileSystem
        {
        public:
            virtual fs::file_status GetPathStatus(const fs::path& path) const = 0;
            virtual Common::RResult GetPathUniqueIdentity(const fs::path& path, U8String& indetity) const = 0;
            //       virtual RfxResult CalcCombinedPath(SlangPathType fromPathType, const U8String& fromPath, const U8String& path, fs::path& pathOut) = 0;
        };

        struct IRfxMutableFileSystem : public IRfxFileSystem
        {
        };

        class OSFileSystem final : public IRfxFileSystem
        {
        public:
            virtual fs::file_status GetPathStatus(const fs::path& path) const override;

            virtual Common::RResult GetPathUniqueIdentity(const fs::path& path, U8String& indetity) const override;
            //        virtual RfxResult CalcCombinedPath(SlangPathType fromPathType, const U8String& fromPath, const U8String& path, fs::path& pathOut) override;
        };
    }
}