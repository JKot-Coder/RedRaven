#pragma once

#include <string>

namespace Common {
    class Stream;
}

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
    public:
        inline static const std::unique_ptr<FileSystem>& Instance() {
            return instance;
        }

        std::shared_ptr<Common::Stream> Open(const std::string &fileName, Mode RW = MODE_READ) const;
    private:
        static std::unique_ptr<FileSystem> instance;
    };

    inline static const std::unique_ptr<FileSystem>& Instance() {
        return FileSystem::Instance();
    }
}


