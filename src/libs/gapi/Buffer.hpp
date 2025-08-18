#pragma once

#include "gapi/GpuResource.hpp"

namespace RR::RenderLoom
{
    class DeviceContext;
}

namespace RR
{
    namespace GAPI
    {
        class Buffer final : public GpuResource
        {
        public:
            using UniquePtr = eastl::unique_ptr<Buffer>;

            static constexpr size_t MaxPossible = std::numeric_limits<size_t>::max();

        public:
            const ShaderResourceView* GetSRV(GpuResourceFormat format, size_t firstElement = 0, size_t numElements = MaxPossible);
            const UnorderedAccessView* GetUAV(GpuResourceFormat format, size_t firstElement = 0, size_t numElements = MaxPossible);

        private:
            static UniquePtr Create(
                const GpuResourceDescription& description,
                IDataBuffer::SharedPtr initialData,
                const std::string& name)
            {
                return UniquePtr(new Buffer(description, initialData, name));
            }

            Buffer(
                const GpuResourceDescription& description,
                IDataBuffer::SharedPtr initialData,
                const std::string& name)
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