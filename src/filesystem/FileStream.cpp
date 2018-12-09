#include <iostream>

#include <common/Exception.hpp>

#include "FileSystem.hpp"
#include "FileStream.hpp"

namespace FileSystem {
    
    FileStream::FileStream(const std::string &fileName)
            : fileName(fileName), fileStream(), mode(Mode::CLOSED) {

    }

    FileStream::~FileStream() {
        if (mode != Mode::CLOSED)
            Close();
    }

    bool FileStream::Open(Mode mode) {
        if (mode == Mode::CLOSED)
            return true;

        switch (mode) {
            case Mode::READ: {
                fileStream.open(fileName, std::ios::in);
            }
                break;
            case Mode::WRITE: {
                fileStream.open(fileName, std::ios::in | std::ios::out);
            }
                break;
            case Mode::APPEND: {
                fileStream.open(fileName, std::ios::in | std::ios::out);
                fileStream.seekg(0, std::ios::end);
            }
                break;
            default:
                throw Common::Exception("Wrong file mode");
        }

        if (fileStream.fail() )
            throw Common::Exception("Error while opening file %s", fileName.c_str());

        this->mode = mode;

        return true;
    }

    void FileStream::Close() {
        if (mode == Mode::CLOSED)
            return;

        fileStream.close();
        mode = Mode::CLOSED;
    }

    int FileStream::GetPosition() {
        if (mode == Mode::CLOSED)
            return 0;

        return fileStream.tellg();
    }

    int FileStream::GetSize() {
        if (mode == Mode::CLOSED)
            return 0;

        auto currentPosition = GetPosition();

        fileStream.seekg(0, fileStream.end);
        auto size = GetPosition();

        SetPosition(currentPosition);

        return size;
    }

    void FileStream::SetPosition(int value) {
        if (mode == Mode::CLOSED)
            throw Common::Exception("Error file %s not opened", fileName.c_str());

        fileStream.seekg(value);
    }

    int FileStream::Read(char *data, int length) {
        if (mode == Mode::CLOSED)
            throw Common::Exception("Error file %s not opened", fileName.c_str());

        int startPosition = GetPosition();

        fileStream.read(data, length);

        if (fileStream.fail())
            throw Common::Exception("Error while reading file %s", fileName.c_str());

        if(fileStream.eof()){
            return GetPosition() - startPosition;
        }

        return length;
    }

    int FileStream::Write(const char *data, int length) {
        if (mode == Mode::CLOSED)
            throw Common::Exception("Error file %s not opened", fileName.c_str());

        fileStream.write(data, length);

        if (fileStream.fail())
            throw Common::Exception("Error while writing file %s", fileName.c_str());

        return length;
    }
}
