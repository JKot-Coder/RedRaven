#include "common/Result.hpp"

#include "common/io/File.hpp"
#include "common/io/FileSystem.hpp"

#include <filesystem>

namespace RR::Common
{
    namespace IO
    {
        std::string FileSystem::CurrentPath() const
        {
            return std::filesystem::current_path().generic_u8string();
        }

        bool FileSystem::IsExist(std::string_view path) const
        {
            return std::filesystem::exists(path);
        }

        RResult FileSystem::Open(std::string_view path, FileOpenMode acess, File& file) const
        {
            return file.Open(path, acess);
        }
    }
}