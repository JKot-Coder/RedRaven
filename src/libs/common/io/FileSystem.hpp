#pragma once

namespace RR::Common
{
    enum class RResult : int32_t;

    namespace IO
    {
        enum class FileOpenMode : uint32_t;
        class File;

        class FileSystem final
        {
        public:
            std::string CurrentPath() const;
            bool IsExist(std::string_view path) const;
            RResult Open(std::string_view path, FileOpenMode acess, File& file)const;
        };
    }
}