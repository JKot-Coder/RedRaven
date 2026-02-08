#pragma once

#include "gapi/GpuResource.hpp"

namespace RR::Render
{
    class DeviceContext;
}

namespace RR::GAPI
{
    class Texture final : public GpuResource
    {
    public:
        static constexpr uint32_t MaxPossible = 0xFFFFFF;

    private:
        Texture(const GpuResourceDesc& desc,
                IDataBuffer::SharedPtr initialData,
                const std::string& name)
            : GpuResource(desc, name)
        {
            UNUSED(initialData);
            if (!desc.IsTexture())
                LOG_FATAL("Wrong Description");
        };

    private:
        friend class Render::DeviceContext;
    };
}