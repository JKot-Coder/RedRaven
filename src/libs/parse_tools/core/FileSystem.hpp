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
            virtual Common::RResult GetPathUniqueIdentity(const fs::path& path, std::string& indetity) const = 0;
            //       virtual RfxResult CalcCombinedPath(SlangPathType fromPathType, const std::string& fromPath, const std::string& path, fs::path& pathOut) = 0;
        };

        struct IRfxMutableFileSystem : public IRfxFileSystem
        {
        };

        class OSFileSystem final : public IRfxFileSystem
        {
        public:
            virtual fs::file_status GetPathStatus(const fs::path& path) const override;

            virtual Common::RResult GetPathUniqueIdentity(const fs::path& path, std::string& indetity) const override;
            //        virtual RfxResult CalcCombinedPath(SlangPathType fromPathType, const std::string& fromPath, const std::string& path, fs::path& pathOut) override;
        };
    }
}