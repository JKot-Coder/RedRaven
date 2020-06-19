#pragma once

#include "common/Stream.hpp"
#include "common/Common.hpp"

#include <fstream>

namespace OpenDemo
{
    namespace FileSystem
    {

        enum class Mode : int8_t;

        class FileStream final : public Common::Stream
        {
        public:
            FileStream(const U8String& fileName);
            virtual ~FileStream() override;

            bool Open(Mode mode);
            void Close();

            virtual inline U8String GetName() const override
            {
                return fileName;
            }

            virtual int64_t GetPosition() override;
            virtual void SetPosition(int64_t value) override;

            virtual int64_t GetSize() override;

            virtual int64_t Read(char* data, int64_t length) override;
            virtual int64_t Write(const char* data, int64_t length) override;

            virtual std::istream* GetNativeStream() override;

        private:
            U8String fileName;
            std::fstream fileStream;
            Mode mode;
        };

    }
}