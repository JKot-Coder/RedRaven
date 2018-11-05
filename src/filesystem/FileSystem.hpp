#pragma once

#include <string>

namespace FileSystem {

    class Stream;

    enum Mode
    {
        MODE_CLOSED,
        MODE_READ,
        MODE_WRITE,
        MODE_APPEND,
        MODE_MAX_ENUM
    };

    class FileSystem {
        const std::shared_ptr<Stream> Open(const std::string &fileName, Mode RW = MODE_READ);
    };
}


