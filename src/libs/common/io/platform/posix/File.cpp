#include "common/io/File.hpp"

#include "common/Result.hpp"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "EASTL/fixed_string.h"

namespace RR::Common::IO
{
    File::~File() { Close(); }

    RResult File::Open(std::string_view path, FileOpenMode accessMode)
    {
        Close();

        if (path.empty())
            return RResult::NotFound;


        int desiredAccess = 0;
        switch (accessMode)
        {
            case FileOpenMode::Read: desiredAccess = O_RDONLY; break;
            case FileOpenMode::ReadWrite: desiredAccess = O_RDWR; break;
            case FileOpenMode::Create: desiredAccess = O_RDWR | O_CREAT; break;
            case FileOpenMode::CreateTruncate: desiredAccess = O_RDWR | O_CREAT | O_TRUNC; break;
            case FileOpenMode::CreateAppend: desiredAccess = O_RDWR | O_CREAT | O_APPEND; break;
            default: return RResult::InvalidArgument;
        }

        eastl::fixed_string<char, 512> pathStr(path.begin(), path.end());

        const int permissions = S_IRWXU | S_IRGRP | S_IROTH; // 744
        int handle = open(pathStr.c_str(), desiredAccess, permissions);

        if (handle < 0)
        {
            switch (errno)
            {
                case EACCES: return RResult::AccessDenied;
                case EEXIST: return RResult::AlreadyExist;
                case ENOENT: return RResult::FileNotFound;
                default: return RResult::CannotOpen;
            }
        }

        handle_ = handle;
        return RResult::Ok;
    }

    void File::Close()
    {
        if (!IsOpen())
            return;

        close(handle_);
        handle_ = InvalidHandle;
    }

    size_t File::Write(const void* buffer, size_t byteSize) const
    {
        if (!IsOpen())
        {
            ASSERT_MSG(false, "File is not opened");
            return 0;
        }

        ssize_t result = write(handle_, buffer, byteSize);
        if (result == -1)
            return 0;

        return result;
    }
}