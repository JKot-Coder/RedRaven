#include "FileSystem.hpp"

#include "FileStream.hpp"

namespace RR
{
    namespace FileSystem
    {
        std::unique_ptr<FileSystem> FileSystem::instance = std::unique_ptr<FileSystem>(new FileSystem());

        std::shared_ptr<Common::Stream> FileSystem::Open(const std::string& fileName, Mode RW) const
        {
            auto* fileStream = new FileStream(fileName);

            fileStream->Open(RW);

            return std::shared_ptr<Common::Stream>(fileStream);
        }
    }
}