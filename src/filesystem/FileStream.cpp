#include "FileStream.hpp"

#include <iostream>

#include <common/Exception.hpp>

#include "FileSystem.hpp"

namespace OpenDemo
{
    namespace FileSystem
    {

        FileStream::FileStream(const std::string& fileName)
            : fileName(fileName)
            , fileStream()
            , mode(Mode::CLOSED)
        {
        }

        FileStream::~FileStream()
        {
            if (mode != Mode::CLOSED)
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
                fileStream.open(fileName, std::ios::in | std::ios::binary);
            }
            break;
            case Mode::WRITE:
            {
                fileStream.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
            }
            break;
            case Mode::APPEND:
            {
                fileStream.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
                fileStream.seekg(0, std::ios::end);
            }
            break;
            default:
                throw Common::Exception("Wrong file mode");
            }

            if (fileStream.fail())
                throw Common::Exception("Error while opening file %s", fileName.c_str());

            mode = openMode;
            return true;
        }

        void FileStream::Close()
        {
            if (mode == Mode::CLOSED)
                return;

            fileStream.close();
            mode = Mode::CLOSED;
        }

        int64_t FileStream::GetPosition()
        {
            if (mode == Mode::CLOSED)
                return 0;

            return fileStream.tellg();
        }

        int64_t FileStream::GetSize()
        {
            if (mode == Mode::CLOSED)
                return 0;

            auto currentPosition = GetPosition();

            fileStream.seekg(0, fileStream.end);
            auto size = GetPosition();

            SetPosition(currentPosition);

            return size;
        }

        void FileStream::SetPosition(int64_t value)
        {
            if (mode == Mode::CLOSED)
                throw Common::Exception("Error file %s not opened", fileName.c_str());

            fileStream.seekg(value);
        }

        int64_t FileStream::Read(char* data, int64_t length)
        {
            if (mode == Mode::CLOSED)
                throw Common::Exception("Error file %s not opened", fileName.c_str());

            int64_t startPosition = GetPosition();

            fileStream.exceptions(fileStream.exceptions() | std::ios::failbit);
            try
            {
                fileStream.read(data, length);
            }
            catch (std::ios_base::failure& e)
            {
                std::cerr << e.what() << "!" << e.code().message() << '\n';
            }

            if (fileStream.fail())
            {

                throw Common::Exception("Error while reading file %s", fileName.c_str());
            }

            if (fileStream.eof())
            {
                return GetPosition() - startPosition;
            }

            return length;
        }

        int64_t FileStream::Write(const char* data, int64_t length)
        {
            if (mode == Mode::CLOSED)
                throw Common::Exception("Error file %s not opened", fileName.c_str());

            fileStream.write(data, length);

            if (fileStream.fail())
                throw Common::Exception("Error while writing file %s", fileName.c_str());

            return length;
        }

        std::istream* FileStream::GetNativeStream()
        {
            return &fileStream;
        }
    }
}