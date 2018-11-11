#pragma once

#include <string>

namespace FileSystem {

    class FileStream;

    enum Mode : int8_t
    {
        MODE_CLOSED,
        MODE_READ,
        MODE_WRITE,
        MODE_APPEND,
        MODE_MAX_ENUM
    };

    class FileSystem {
        const std::shared_ptr<FileStream> Open(const std::string &fileName, Mode RW = MODE_READ);
    };
}


