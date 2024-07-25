#include "common/Result.hpp"

#include "common/io/File.hpp"
#include "common/io/FileSystem.hpp"

#include <filesystem>

namespace RR::Common
{
    namespace IO
    {
        bool FileSystem::IsExist(std::string_view path)
        {
            return std::filesystem::exists(path);
        }

        RResult FileSystem::Open(std::string_view path, FileOpenMode acess, File& file)
        {
            return file.Open(path, acess);
        }
    }
}