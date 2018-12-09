#pragma once

#include <string>

namespace Common {
    class Stream;
}

namespace FileSystem {

    class FileStream;

    enum class Mode : int8_t
    {
        CLOSED,
        READ,
        WRITE,
        APPEND
    };

    class FileSystem {
    public:
        inline static const std::unique_ptr<FileSystem>& Instance() {
            return instance;
        }

        std::shared_ptr<Common::Stream> Open(const std::string &fileName, Mode RW = Mode::READ) const;
    private:
        static std::unique_ptr<FileSystem> instance;
    };

    inline static const std::unique_ptr<FileSystem>& Instance() {
        return FileSystem::Instance();
    }
}


