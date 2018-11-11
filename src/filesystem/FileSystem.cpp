#include "FileSystem.hpp"


namespace FileSystem {

    const std::shared_ptr<FileStream> FileSystem::Open(const std::string &fileName, Mode RW) {
        (void) fileName;
        (void) RW;

        return std::shared_ptr<FileStream>();
    }

}
