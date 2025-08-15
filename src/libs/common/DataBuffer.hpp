#pragma once

namespace RR::Common
{
    class IDataBuffer
    {
    public:
        using SharedPtr = eastl::shared_ptr<IDataBuffer>;

    public:
        virtual ~IDataBuffer() = default;

        virtual size_t Size() const = 0;
        virtual void* Data() const = 0;
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

        size_t Size() const override { return size_; }
        void* Data() const override { return data_; }

    private:
        size_t size_;
        std::byte* data_;
    };
}