#include "FileStream.hpp"

#include "FileSystem.hpp"

namespace FileSystem {

    std::unique_ptr<FileSystem> FileSystem::instance = std::unique_ptr<FileSystem>(new FileSystem());

    Common::Stream* FileSystem::Open(const std::string &fileName, Mode RW) {
        auto* fileStream = new FileStream(fileName);

        fileStream->Open(RW);

        return fileStream;
    }

}
