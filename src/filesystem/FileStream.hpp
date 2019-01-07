#pragma once

#include "common/Stream.hpp"

#include <string>
#include <cstdint>
#include <fstream>

namespace FileSystem {

    enum class Mode : int8_t;

    class FileStream final: public Common::Stream {
    public:
        FileStream(const std::string &fileName);
        virtual ~FileStream() override;

        bool Open(Mode mode);
        void Close();

        virtual inline std::string GetName() const override  {
            return fileName;
        }

        virtual int32_t GetPosition() override;
        virtual void SetPosition(int32_t value) override;

        virtual int32_t GetSize() override;

        virtual int32_t Read(char *data, int32_t length) override;
        virtual int32_t Write(const char *data, int32_t length) override;

        virtual std::istream* GetNativeStream() override;
    private:
        std::string fileName;
        std::fstream fileStream;
        Mode mode;
    };



}

