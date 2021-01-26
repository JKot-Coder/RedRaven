#pragma once

#include "gapi/GpuResource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {

        struct BufferDescription
        {
            GpuResourceFormat format = GpuResourceFormat::Unknown;
            uint32_t size = 0;

            static BufferDescription Create(uint32_t size, GpuResourceFormat format = GpuResourceFormat::Unknown)
            {
                return BufferDescription(size, format);
            }

        private:
            BufferDescription(uint32_t size, GpuResourceFormat format)
                : size(size),
                  format(format)
            {
            }
        };

        class Buffer final : public GpuResource
        {
        public:
            using SharedPtr = std::shared_ptr<Buffer>;
            using SharedConstPtr = std::shared_ptr<const Buffer>;

            static constexpr uint32_t MaxPossible = 0xFFFFFF;

        public:
            std::shared_ptr<ShaderResourceView> GetSRV(uint32_t firstElement = 0, uint32_t numElements = MaxPossible);
            std::shared_ptr<UnorderedAccessView> GetUAV(uint32_t firstElement = 0, uint32_t numElements = MaxPossible);

            const BufferDescription& GetDescription() const { return description_; }

        private:
            template <class Deleter>
            static SharedPtr Create(
                const Buffer& description,
                GpuResourceBindFlags bindFlags,
                const U8String& name,
                Deleter)
            {
                return SharedPtr(new Buffer(description, bindFlags, name), Deleter());
            }

            Buffer(const BufferDescription& description, GpuResourceBindFlags bindFlags, const U8String& name);

        private:
            BufferDescription description_;

            friend class Render::RenderContext;
        };
    }
}