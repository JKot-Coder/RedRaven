#include <iostream>

#include <common/Exception.hpp>

#include "FileSystem.hpp"
#include "FileStream.hpp"

namespace FileSystem {
    
    FileStream::FileStream(const std::string &fileName)
            : fileName(fileName), fileStream(), mode(MODE_CLOSED) {

    }

    FileStream::~FileStream() {
        if (mode != MODE_CLOSED)
            Close();
    }

    bool FileStream::Open(Mode mode) {
        if (mode == MODE_CLOSED)
            return true;

        switch (mode) {
            case MODE_READ: {
                fileStream.open(fileName, std::ios::out);
            }
                break;
            case MODE_WRITE: {
                fileStream.open(fileName, std::ios::in | std::ios::out);
            }
                break;
            case MODE_APPEND: {
                fileStream.open(fileName, std::ios::in | std::ios::out);
                fileStream.seekg(0, std::ios::end);
            }
                break;
            default:
                Common::Exception("Wrong file mode");
                return false;
        }

        if (fileStream.fail() )
            throw Common::Exception("Error while opening file %s", fileName.c_str());

        this->mode = mode;

        return true;
    }

    void FileStream::Close() {
        if (mode == MODE_CLOSED)
            return;

        fileStream.close();
        mode = MODE_CLOSED;
    }

    int32_t FileStream::GetPosition() {
        if (mode == MODE_CLOSED)
            return 0;

        return fileStream.tellg();
    }

    int32_t FileStream::GetSize() {
        if (mode == MODE_CLOSED)
            return 0;

        auto currentPosition = GetPosition();

        fileStream.seekg(0, std::ios_base::end);
        auto size = GetPosition();

        SetPosition(currentPosition);

        return size;
    }

    void FileStream::SetPosition(int32_t value) {
        if (mode == MODE_CLOSED)
            throw Common::Exception("Error file %s not opened", fileName.c_str());

        fileStream.seekg(value);
    }

    int32_t FileStream::Read(char *data, int32_t length) {
        if (mode == MODE_CLOSED)
            throw Common::Exception("Error file %s not opened", fileName.c_str());

        int32_t startPosition = GetPosition();

        fileStream.read(data, length);

        if (fileStream.fail())
            throw Common::Exception("Error while reading file %s", fileName.c_str());

        if(fileStream.eof()){
            return GetPosition() - startPosition;
        }

        return length;
    }

    int32_t FileStream::Write(const char *data, int32_t length) {
        if (mode == MODE_CLOSED)
            throw Common::Exception("Error file %s not opened", fileName.c_str());

        fileStream.write(data, length);

        if (fileStream.fail())
            throw Common::Exception("Error while writing file %s", fileName.c_str());

        return length;
    }
}
