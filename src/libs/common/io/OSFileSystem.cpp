
#include "common/OSFileSystem.hpp"

#include "Result.hpp"

namespace RR::Common::IO
{
    fs::file_status OSFileSystem::GetPathStatus(const fs::path& path) const
    {
        return fs::status(path);
    }

    RResult OSFileSystem::GetPathUniqueIdentity(const fs::path& path, U8String& indetity) const
    {
        std::error_code errorcode;
        indetity = fs::canonical(path, errorcode).generic_u8string();

        if (!errorcode)
            return RResult::Ok;

        if (errorcode == std::errc::no_such_file_or_directory)
            return RResult::NotFound;

        return RResult::Fail;
    }
}