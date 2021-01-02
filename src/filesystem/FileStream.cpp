#include "FileStream.hpp"

#include <iostream>

#include <common/Exception.hpp>

#include "FileSystem.hpp"

namespace OpenDemo
{
    namespace FileSystem
    {
        FileStream::FileStream(const U8String& fileName)
            : _fileName(fileName), _fileStream(), _mode(Mode::CLOSED)
        {
        }

        FileStream::~FileStream()
        {
            if (_mode != Mode::CLOSED)
                Close();
        }

        bool FileStream::Open(Mode openMode)
        {
            if (openMode == Mode::CLOSED)
                return true;

            switch (openMode)
            {
            case Mode::READ:
            {
                _fileStream.open(_fileName, std::ios::in | std::ios::binary);
            }
            break;
            case Mode::WRITE:
            {
                _fileStream.open(_fileName, std::ios::in | std::ios::out | std::ios::binary);
            }
            break;
            case Mode::APPEND:
            {
                _fileStream.open(_fileName, std::ios::in | std::ios::out | std::ios::binary);
                _fileStream.seekg(0, std::ios::end);
            }
            break;
            default:
                throw Common::Exception("Wrong file mode");
            }

            if (_fileStream.fail())
                throw Common::Exception(fmt::format(FMT_STRING("Error while opening file {}"), _fileName.c_str()));

            _mode = openMode;
            return true;
        }

        void FileStream::Close()
        {
            if (_mode == Mode::CLOSED)
                return;

            _fileStream.close();
            _mode = Mode::CLOSED;
        }

        int64_t FileStream::GetPosition()
        {
            if (_mode == Mode::CLOSED)
                return 0;

            return _fileStream.tellg();
        }

        int64_t FileStream::GetSize()
        {
            if (_mode == Mode::CLOSED)
                return 0;

            auto currentPosition = GetPosition();

            _fileStream.seekg(0, _fileStream.end);
            auto size = GetPosition();

            SetPosition(currentPosition);

            return size;
        }

        void FileStream::SetPosition(int64_t value)
        {
            if (_mode == Mode::CLOSED)
                throw Common::Exception(fmt::format(FMT_STRING("Error file {} not opened"), _fileName.c_str()));

            _fileStream.seekg(value);
        }

        int64_t FileStream::Read(char* data, int64_t length)
        {
            if (_mode == Mode::CLOSED)
                throw Common::Exception(fmt::format(FMT_STRING("Error file {} not opened"), _fileName.c_str()));

            int64_t startPosition = GetPosition();

            _fileStream.exceptions(_fileStream.exceptions() | std::ios::failbit);
            try
            {
                _fileStream.read(data, length);
            }
            catch (std::ios_base::failure& e)
            {
                std::cerr << e.what() << "!" << e.code().message() << '\n';
            }

            if (_fileStream.fail())
            {
                throw Common::Exception(fmt::format(FMT_STRING("Error while reading file {}"), _fileName.c_str()));
            }

            if (_fileStream.eof())
            {
                return GetPosition() - startPosition;
            }

            return length;
        }

        int64_t FileStream::Write(const char* data, int64_t length)
        {
            if (_mode == Mode::CLOSED)
                throw Common::Exception(fmt::format(FMT_STRING("Error file {} not opened"), _fileName.c_str()));

            _fileStream.write(data, length);

            if (_fileStream.fail())
                throw Common::Exception(fmt::format(FMT_STRING("Error while writing file {}"), _fileName.c_str()));

            return length;
        }

        std::istream* FileStream::GetNativeStream()
        {
            return &_fileStream;
        }
    }
}