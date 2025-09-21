#pragma once

#include "gapi/GpuResource.hpp"

namespace RR::Render
{
    class DeviceContext;
}

namespace RR
{
    namespace GAPI
    {
        struct BufferData
        {
            const void* data = nullptr;
            size_t size = 0;

        public:
            constexpr BufferData() = default;
            constexpr BufferData(const void* data, size_t size) : data(data), size(size)
            {
                ASSERT(data);
            }
        };

        class Buffer final : public GpuResource
        {
        public:
            using SharedPtr = eastl::shared_ptr<Buffer>;
            using SharedConstPtr = eastl::shared_ptr<const Buffer>;

            static constexpr size_t MaxPossible = std::numeric_limits<size_t>::max();

        public:
            const ShaderResourceView* GetSRV(GpuResourceFormat format, size_t firstElement = 0, size_t numElements = MaxPossible);
            const UnorderedAccessView* GetUAV(GpuResourceFormat format, size_t firstElement = 0, size_t numElements = MaxPossible);

        private:
            EASTL_FRIEND_MAKE_SHARED;

            static SharedPtr Create(const GpuResourceDesc& desc, const std::string& name)
            {
                return eastl::make_shared<Buffer>(desc, name);
            }

            Buffer(const GpuResourceDesc& desc, const std::string& name) : GpuResource(desc, name)
            {
                if (!desc.IsBuffer())
                    LOG_FATAL("Wrong Description");
            }

        private:
            friend class Render::DeviceContext;
        };
    }
}