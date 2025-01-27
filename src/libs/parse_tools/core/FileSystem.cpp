#include "FileSystem.hpp"

#include "common/Result.hpp"

namespace RR
{
    namespace Rfx
    {
        fs::file_status OSFileSystem::GetPathStatus(const fs::path& path) const
        {
            return fs::status(path);
        }

        RfxResult OSFileSystem::GetPathUniqueIdentity(const fs::path& path, std::string& indetity) const
        {
            std::error_code errorcode;
            indetity = fs::canonical(path, errorcode).generic_u8string();

            if (!errorcode)
                return RfxResult::Ok;

            if (errorcode == std::errc::no_such_file_or_directory)
                return RfxResult::NotFound;

            return RfxResult::Fail;
        }

        /* RfxResult OSFileSystem::CalcCombinedPath(const fs::path& fromPath, const std::string& path, fs::path& pathOut)
        {
            // Don't need to fix delimiters - because combine path handles both path delimiter types
            switch (fromPathType)
            {
                case SLANG_PATH_TYPE_FILE:
                {
                    pathOut = fs::path(fromPath).parent_path().concat(path);
                    break;
                }
                case SLANG_PATH_TYPE_DIRECTORY:
                {
                    pathOut = fs::path(fromPath).concat(path);
                    break;
                }
            }

            return RfxResult::Ok;
        }*/
    }
}