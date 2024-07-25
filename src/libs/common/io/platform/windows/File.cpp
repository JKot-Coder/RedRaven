#include "common/io/File.hpp"
#include "common/StringEncoding.hpp"
#include "common/Result.hpp"

#include <windows.h>

#include"EASTL\fixed_string.h"

namespace RR::Common::IO
{
    File::~File() { Close(); }

    RResult File::Open(const std::string_view& path, FileOpenMode openMode)
    {
        Close();

        if (path.empty())
            return RResult::NotFound;

        const auto createFile = [](const WCHAR* path, FileOpenMode openMode) {
            const auto sysCreateFile = [](const WCHAR* path, bool writeAcess, DWORD dwCreationDisposition) {
                return ::CreateFileW(path, GENERIC_READ | (writeAcess ? GENERIC_WRITE : 0), FILE_SHARE_READ | FILE_SHARE_WRITE | (writeAcess ? FILE_SHARE_DELETE : 0), nullptr, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);
            };

            switch (openMode)
            {
                 // TODO: support FILE_ATTRIBUTE_TEMPORARY for optimization
                case FileOpenMode::Read: return sysCreateFile(path, false, OPEN_EXISTING); // Opens a file for read only, only if it exists.
                case FileOpenMode::ReadWrite: return sysCreateFile(path, true, OPEN_EXISTING); // Opens a file for read and write, only if it exists.
                case FileOpenMode::Create: return sysCreateFile(path, true, OPEN_ALWAYS); // Opens a file for read and write. Create empty file if it doesn't exist.
                case FileOpenMode::CreateTruncate: return sysCreateFile(path, true, CREATE_ALWAYS); // Opens a file for read and write, truncated. Create empty file if it doesn't exist.
                case FileOpenMode::CreateAppend: { // Opens a file for read and write at it's end. Create empty file if it doesn't exist.
                    HANDLE handle = sysCreateFile(path, true, OPEN_ALWAYS);
                    LARGE_INTEGER position;
                    LARGE_INTEGER offset;
                    ZeroMemory(&offset, sizeof offset);
                    ::SetFilePointerEx(handle, offset, &position, FILE_END);
                    return handle;
                }
                default: return INVALID_HANDLE_VALUE;
            }
        };

        eastl::fixed_string<WCHAR, 512> wpath;
        Common::StringEncoding::UTF8ToWide(path.begin(), path.end(), eastl::back_inserter(wpath));

        HANDLE handle = createFile(wpath.c_str(), openMode);
        if (handle == INVALID_HANDLE_VALUE)
        {
            switch (GetLastError())
            {
                case ERROR_SHARING_VIOLATION:
                case ERROR_UNABLE_TO_REMOVE_REPLACED:
                case ERROR_UNABLE_TO_MOVE_REPLACEMENT:
                case ERROR_UNABLE_TO_MOVE_REPLACEMENT_2:
                    return RResult::AccessDenied;
                case ERROR_ALREADY_EXISTS:
                case ERROR_FILE_EXISTS:
                    return RResult::AlreadyExist;
                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                    return RResult::FileNotFound;
                case ERROR_ACCESS_DENIED:
                case ERROR_LOCK_VIOLATION:
                     return RResult::AccessDenied;
                case ERROR_TOO_MANY_OPEN_FILES:
                    return RResult::OutOfMemory;
                case ERROR_OUTOFMEMORY:
                case ERROR_NOT_ENOUGH_MEMORY:
                    return RResult::OutOfMemory;
                case ERROR_HANDLE_DISK_FULL:
                case ERROR_DISK_FULL:
                case ERROR_DISK_RESOURCES_EXHAUSTED:
                    return RResult::DiskFull;
                case ERROR_USER_MAPPED_FILE:
                  return RResult::CannotOpen;
                case ERROR_NOT_READY: // The device is not ready.
                case ERROR_SECTOR_NOT_FOUND: // The drive cannot find the sector requested.
                case ERROR_GEN_FAILURE: // A device ... is not functioning.
                case ERROR_DEV_NOT_EXIST: // Net resource or device is no longer available.
                case ERROR_IO_DEVICE:
                case ERROR_DISK_OPERATION_FAILED:
                case ERROR_FILE_CORRUPT: // File or directory is corrupted and unreadable.
                case ERROR_DISK_CORRUPT: // The disk structure is corrupted and unreadable.
                    return RResult::CannotOpen;
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

        CloseHandle(handle_);
        handle_ = InvalidHandle;
    }

    size_t File::Write(const void* buffer, size_t byteSize) const
    {
        if (!IsOpen())
        {
            ASSERT_MSG(false, "File is not opened");
            return 0;
        }

        DWORD result;
        if (!WriteFile(handle_, buffer, byteSize, &result, nullptr))
            return 0;

        return result;
    }
}