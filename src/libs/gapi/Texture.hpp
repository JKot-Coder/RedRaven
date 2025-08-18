#pragma once

#include "gapi/GpuResource.hpp"

namespace RR::RenderLoom
{
    class DeviceContext;
}

namespace RR::GAPI
{
    class Texture final : public GpuResource
    {
    public:
        using UniquePtr = eastl::unique_ptr<Texture>;

        static constexpr uint32_t MaxPossible = 0xFFFFFF;

    public:
        const ShaderResourceView* GetSRV(uint32_t mipLevel = 0, uint32_t mipCount = MaxPossible, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        const RenderTargetView* GetRTV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        const DepthStencilView* GetDSV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        const UnorderedAccessView* GetUAV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);

    private:
        static UniquePtr Create(
            const GpuResourceDescription& description,
            IDataBuffer::SharedPtr initialData,
            const std::string& name)
        {
            return UniquePtr(new Texture(description, initialData, name));
        }

        Texture(const GpuResourceDescription& description,
                IDataBuffer::SharedPtr initialData,
                const std::string& name)
            : GpuResource(description, initialData, name)
        {
            if (!description.IsTexture())
                LOG_FATAL("Wrong Description");
        };

    private:
        friend class Render::DeviceContext;
        friend class RenderLoom::DeviceContext;
    };
}