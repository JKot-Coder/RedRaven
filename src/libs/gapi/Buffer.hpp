#pragma once

#include "gapi/GpuResource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        class Buffer final : public GpuResource
        {
        public:
            using SharedPtr = std::shared_ptr<Buffer>;
            using SharedConstPtr = std::shared_ptr<const Buffer>;

            static constexpr uint32_t MaxPossible = 0xFFFFFF;

        public:
            std::shared_ptr<ShaderResourceView> GetSRV(GpuResourceFormat format, uint32_t firstElement = 0, uint32_t numElements = MaxPossible);
            std::shared_ptr<UnorderedAccessView> GetUAV(GpuResourceFormat format, uint32_t firstElement = 0, uint32_t numElements = MaxPossible);

        private:
            static SharedPtr Create(
                const GpuResourceDescription& description,
                GpuResourceCpuAccess cpuAccess,
                const U8String& name)
            {
                return SharedPtr(new Buffer(description, cpuAccess, name));
            }

            Buffer(const GpuResourceDescription& description, GpuResourceCpuAccess cpuAccess, const U8String& name)
                : GpuResource(description, cpuAccess, name) {};

        private:
            friend class Render::DeviceContext;
        };
    }
}