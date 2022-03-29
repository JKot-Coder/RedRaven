#include "include/rfx.hpp"

#include "core/RefObject.hpp"

#include <string>

namespace RR::Rfx
{
    class Blob : IBlob, RefObject
    {
    public:
        Blob(const std::string& data) : buffer_(data) { }

        uint32_t addRef() override { return addReference(); }
        uint32_t release() override { return releaseReference(); }

        void const* GetBufferPointer() const override { return buffer_.c_str(); }
        size_t GetBufferSize() const override { return buffer_.size(); }

    private:
        std::string buffer_;
    };
}