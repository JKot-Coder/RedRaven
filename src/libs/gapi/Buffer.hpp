#pragma once

#include "gapi/GpuResource.hpp"

namespace RR
{
    namespace GAPI
    {
        struct BufferDescription
        {
        public:
            static BufferDescription Buffer(uint32_t size, GpuResourceFormat format = GpuResourceFormat::Unknown, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default)
            {
                return BufferDescription(size, format == GpuResourceFormat::Unknown ? 1 : GpuResourceFormatInfo::GetBlockSize(format), GpuResourceFormat::Unknown, bindFlags, usage);
            }

            static BufferDescription StructuredBuffer(uint32_t size, uint32_t structSize, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default)
            {
                return BufferDescription(size, structSize, GpuResourceFormat::Unknown, bindFlags, usage);
            }

            inline friend bool operator==(const BufferDescription& lhs, const BufferDescription& rhs)
            {
                static_assert(sizeof(BufferDescription) == 5 * sizeof(uint32_t), "Check for tighly packed structure");
                return std::memcmp(&lhs, &rhs, sizeof(BufferDescription)) == 0;
            }
            inline friend bool operator!=(const BufferDescription& lhs, const BufferDescription& rhs) { return !(lhs == rhs); }

        public:
            bool IsTyped() const { return format != GpuResourceFormat::Unknown; }
            uint32_t GetNumElements() const { return size / stride; }
            uint32_t GetNumSubresources() const { return 1; }
            bool IsValid() const;

        private:
            BufferDescription(uint32_t size, uint32_t stride, GpuResourceFormat format, GpuResourceBindFlags bindFlags, GpuResourceUsage usage)
                : size(size),
                  stride(stride),
                  format(format),
                  bindFlags(bindFlags),
                  usage(usage)
            {
                ASSERT(IsValid());
            }

        public:
            uint32_t size = 1;
            uint32_t stride = 1;
            GpuResourceFormat format = GpuResourceFormat::Unknown;
            GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource;
            GpuResourceUsage usage = GpuResourceUsage::Default;
        };

        class Buffer final : public GpuResource
        {
        public:
            using SharedPtr = std::shared_ptr<Buffer>;
            using SharedConstPtr = std::shared_ptr<const Buffer>;

            static constexpr uint32_t MaxPossible = 0xFFFFFF;

        public:
            std::shared_ptr<ShaderResourceView> GetSRV(GpuResourceFormat format, uint32_t firstElement = 0, uint32_t numElements = MaxPossible);
            std::shared_ptr<UnorderedAccessView> GetUAV(GpuResourceFormat format, uint32_t firstElement = 0, uint32_t numElements = MaxPossible);

            const BufferDescription& GetDescription() const { return description_; }

        private:
            static SharedPtr Create(
                const BufferDescription& description,
                IDataBuffer::SharedPtr initialData,
                const U8String& name)
            {
                return SharedPtr(new Buffer(description, initialData, name));
            }

            Buffer(const BufferDescription& description, IDataBuffer::SharedPtr initialData, const U8String& name)
                : GpuResource(GpuResourceType::Buffer, initialData, name),
                  description_(description) {};

        private:
            BufferDescription description_;

            friend class Render::DeviceContext;
        };
    }
}