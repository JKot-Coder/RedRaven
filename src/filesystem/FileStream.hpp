#pragma once

#include "common/Stream.hpp"

#include <string>
#include <cstdint>
#include <fstream>

namespace FileSystem {

    enum Mode : int8_t;

    class FileStream : public Common::Stream {
    public:
        FileStream(const std::string &fileName);
        ~FileStream();

        bool Open(Mode mode);
        void Close();

        virtual int32_t GetPosition() override;
        virtual void SetPosition(int32_t value) override;

        virtual int32_t Read(char *data, int32_t length) override;
        virtual int32_t Write(const char *data, int32_t length) override;
    private:
        std::string fileName;
        std::fstream fileStream;
        Mode mode;
    };



}

