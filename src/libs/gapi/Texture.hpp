#pragma once

#include "gapi/GpuResource.hpp"

namespace RR::GAPI
{
    class Texture final : public GpuResource
    {
    public:
        using SharedPtr = std::shared_ptr<Texture>;
        using SharedConstPtr = std::shared_ptr<const Texture>;

        static constexpr uint32_t MaxPossible = 0xFFFFFF;

    public:
        std::shared_ptr<ShaderResourceView> GetSRV(uint32_t mipLevel = 0, uint32_t mipCount = MaxPossible, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        std::shared_ptr<RenderTargetView> GetRTV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        std::shared_ptr<DepthStencilView> GetDSV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        std::shared_ptr<UnorderedAccessView> GetUAV(uint32_t mipLevel, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);

    private:
        static SharedPtr Create(
            const GpuResourceDescription& description,
            IDataBuffer::SharedPtr initialData,
            const std::string& name)
        {
            return SharedPtr(new Texture(description, initialData, name));
        }

        Texture(const GpuResourceDescription& description, IDataBuffer::SharedPtr initialData, const std::string& name)
            : GpuResource(description, initialData, name)
        {
            if (!description.IsTexture())
                LOG_FATAL("Wrong Description");
        };

    private:
        friend class Render::DeviceContext;
    };
}