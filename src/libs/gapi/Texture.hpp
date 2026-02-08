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

    public:
    /*
        const ShaderResourceView* GetSRV(uint32_t mipLevel = 0, uint32_t mipCount = MaxPossible, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        const RenderTargetView* GetRTV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        const DepthStencilView* GetDSV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        const UnorderedAccessView* GetUAV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
*/
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
        friend class Render::DeviceContext;
    };
}