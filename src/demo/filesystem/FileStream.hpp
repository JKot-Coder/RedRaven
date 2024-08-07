#pragma once

#include "common/Stream.hpp"

#include <fstream>

namespace RR
{
    namespace FileSystem
    {
        enum class Mode : int8_t;

        class FileStream final : public Common::Stream
        {
        public:
            FileStream(const std::string& fileName);
            virtual ~FileStream() override;

            bool Open(Mode mode);
            void Close();

            virtual inline std::string GetName() const override
            {
                return _fileName;
            }

            virtual int64_t GetPosition() override;
            virtual void SetPosition(int64_t value) override;

            virtual int64_t GetSize() override;

            virtual int64_t Read(char* data, int64_t length) override;
            virtual int64_t Write(const char* data, int64_t length) override;

            virtual std::istream* GetNativeStream() override;

        private:
            std::string _fileName;
            std::fstream _fileStream;
            Mode _mode;
        };
    }
}