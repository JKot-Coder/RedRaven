#include "common/io/File.hpp"

#include "common/Result.hpp"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

namespace RR::Common::IO
{
    File::~File()
    {
        if (IsOpen())
            Close();
    }

    RResult File::Open(const fs::path& path, FileAccessMode accessMode)
    {
        int permissions = S_IRWXU | S_IRGRP | S_IROTH; // 744

        if (IsOpen())
        {
            ASSERT_MSG(false, "File aready opened");
            return RResult::Unexpected;
        }

        if (path.empty())
            return RResult::NotFound;

        bool readMode = IsSet(accessMode, FileAccessMode::Read);
        bool writeMode = IsSet(accessMode, FileAccessMode::Write);
        bool openMode = IsSet(accessMode, FileAccessMode::Open);
        bool createMode = IsSet(accessMode, FileAccessMode::Create);
        bool appendMode = IsSet(accessMode, FileAccessMode::Append);
        bool truncateMode = IsSet(accessMode, FileAccessMode::Truncate);

        int desiredAccess = 0;
        {
            if (!readMode && !writeMode)
                return RResult::InvalidArgument;

            desiredAccess = (writeMode && readMode) ? O_RDWR : (writeMode ? O_WRONLY : O_RDONLY);
        }

        {
            if (!openMode && !createMode)
                return RResult::InvalidArgument;

            desiredAccess |= createMode ? O_CREAT : 0;
            desiredAccess |= openMode ? 0 : O_EXCL;
        }

        {
            if (appendMode && truncateMode)
                return RResult::InvalidArgument;

            if (appendMode && !writeMode)
                return RResult::InvalidArgument;

            desiredAccess |= appendMode ? O_APPEND : 0;
            desiredAccess |= truncateMode ? O_TRUNC : 0;
        }

        int handle = open(path.c_str(), desiredAccess, permissions);

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