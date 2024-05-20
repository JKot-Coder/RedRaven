#pragma once

#include "common/EnumClassOperators.hpp"

#include <filesystem>

namespace RR::Common
{
    enum class RResult : int32_t;
    namespace fs = std::filesystem;

    namespace IO
    {
        enum class FileAccessMode : uint32_t;
        ENUM_CLASS_BITWISE_OPS(FileAccessMode);

        enum class FileAccessMode : uint32_t
        {
            Read = 1 << 0,
            Write = 1 << 1,
            Open = 1 << 2,
            Create = 1 << 3,
            Append = 1 << 4,
            Truncate = 1 << 5,

            ReadWrite = Read | Write,
            OpenCreate = Open | Create
        };

        class File
        {
        public:
            ~File();

            RResult Open(const fs::path& path, FileAccessMode accessMode);
            void Close();

            size_t Write(const void* buffer, size_t byteSize) const;

            bool IsOpen() const { return handle_ != InvalidHandle; }

        private:
            static constexpr int InvalidHandle = -1;
            int handle_ = InvalidHandle;
        };
    }
}