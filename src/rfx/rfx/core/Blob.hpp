#include "rfx.hpp"

#include "rfx/core/RefObject.hpp"

#include <string>

namespace RR::Rfx
{
    class StringBlob final : public IBlob, RefObject
    {
    public:
        StringBlob() = default;
        StringBlob(const std::string& data) : buffer_(data) { }

        uint32_t AddRef() override { return addReference(); }
        uint32_t Release() override { return releaseReference(); }

        std::string& GetString() { return buffer_; };
        void const* GetBufferPointer() const override { return static_cast<void const*>(buffer_.data()); }
        size_t GetBufferSize() const override { return buffer_.size(); }

    private:
        std::string buffer_;
    };

    class BinaryBlob final : IBlob, RefObject
    {
    public:
        BinaryBlob(const std::string& data)
        {
            buffer_.reserve(data.size() + 1);
            std::copy(data.begin(), data.end(), std::back_inserter(buffer_));

            buffer_.emplace_back('\0');
        }

        BinaryBlob(const void* data, size_t size)
        {
            buffer_.reserve(size);
            std::copy(static_cast<const char*>(data), static_cast<const char*>(data) + size, std::back_inserter(buffer_));
        }

        uint32_t AddRef() override { return addReference(); }
        uint32_t Release() override { return releaseReference(); }

        void const* GetBufferPointer() const override { return static_cast<void const*>(buffer_.data()); }
        size_t GetBufferSize() const override { return buffer_.size(); }

    private:
        std::vector<char> buffer_;
    };
}