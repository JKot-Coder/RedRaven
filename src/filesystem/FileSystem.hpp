#pragma once

#include <string>

#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#endif

namespace OpenDemo
{
    namespace Common
    {
        class Stream;
    }

    namespace FileSystem
    {

        class FileStream;

        enum class Mode : int8_t
        {
            CLOSED,
            READ,
            WRITE,
            APPEND
        };

        class FileSystem
        {
        public:
            inline static const std::unique_ptr<FileSystem>& Instance()
            {
                return instance;
            }

            std::shared_ptr<Common::Stream> Open(const std::string& fileName, Mode RW = Mode::READ) const;

        private:
            static std::unique_ptr<FileSystem> instance;
        };

        inline static const std::unique_ptr<FileSystem>& Instance()
        {
            return FileSystem::Instance();
        }
    }

}