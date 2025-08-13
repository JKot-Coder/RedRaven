#pragma once

#include "gapi/GpuResource.hpp"

namespace RR
{
    namespace GAPI
    {
        class Buffer final : public GpuResource
        {
        public:
            using SharedPtr = eastl::shared_ptr<Buffer>;
            using SharedConstPtr = eastl::shared_ptr<const Buffer>;

            static constexpr size_t MaxPossible = std::numeric_limits<size_t>::max();

        public:
            eastl::shared_ptr<ShaderResourceView> GetSRV(GpuResourceFormat format, size_t firstElement = 0, size_t numElements = MaxPossible);
            eastl::shared_ptr<UnorderedAccessView> GetUAV(GpuResourceFormat format, size_t firstElement = 0, size_t numElements = MaxPossible);

        private:
            static SharedPtr Create(
                const GpuResourceDescription& description,
                IDataBuffer::SharedPtr initialData,
                const std::string& name)
            {
                return SharedPtr(new Buffer(description, initialData, name));
            }

            Buffer(const GpuResourceDescription& description, IDataBuffer::SharedPtr initialData, const std::string& name)
                : GpuResource(description, initialData, name)
            {
                if (!description.IsBuffer())
                    LOG_FATAL("Wrong Description");
            }

        private:
            friend class Render::DeviceContext;
        };
    }
}