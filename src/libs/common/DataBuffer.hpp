#pragma once

namespace RR::Common
{
    class IDataBuffer
    {
    public:
        using SharedPtr = std::shared_ptr<IDataBuffer>;

    public:
        virtual ~IDataBuffer() = default;

        virtual size_t Size() = 0;
        virtual void* Data() = 0;
    };

    class DataBuffer final : public IDataBuffer
    {
    public:
        DataBuffer(size_t size, const void* initialData = nullptr) : size_(size)
        {
            data_ = new std::byte[size];

            if (initialData != nullptr)
                memcpy(data_, initialData, size);
        }
        ~DataBuffer() { delete[] data_; }

        size_t Size() override { return size_; }
        void* Data() override { return data_; }

    private:
        size_t size_;
        std::byte* data_;
    };
}