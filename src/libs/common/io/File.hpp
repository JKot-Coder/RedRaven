#pragma once

#include "common/EnumClassOperators.hpp"

#if OS_WINDOWS
#include "platform/windows/File.hpp"
#else
#include "platform/posix/File.hpp"
#endif

namespace RR::Common
{
    enum class RResult : int32_t;

    namespace IO
    {
        enum class FileOpenMode : uint32_t
        {
            Read = 1,       // Opens a file for read only, only if it exists.
            ReadWrite,      // Opens a file for read and write, only if it exists.
            Create,         // Opens a file for read and write. Create empty file if it doesn't exist.
            CreateTruncate, // Opens a file for read and write, truncated. Create empty file if it doesn't exist.
            CreateAppend    // Opens a file for read and write at it's end. Create empty file if it doesn't exist.
        };

        class File final : protected FileData
        {
        public:
            ~File();

            RResult Open(std::string_view path, FileOpenMode accessMode);
            void Close();
            bool IsOpen() const { return handle_ != InvalidHandle; }

            size_t Write(const void* buffer, size_t byteSize) const;
            size_t Read(void* buffer, size_t byteSize) const;
        };
    }
}