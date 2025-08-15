#pragma once

#include "gapi/Texture.hpp"
#include "RefCntAutoPtr.hpp"

#include "GraphicsTypes.h"

namespace DL = ::Diligent;
namespace Diligent
{
    class IRenderDevice;
    class ITexture;
    class IBuffer;
}

namespace RR::GAPI::Diligent
{
    class GpuResourceImpl final : public RR::GAPI::IGpuResource
    {
    public:
        GpuResourceImpl() = default;
        ~GpuResourceImpl() override;

        void Init(const DL::RefCntAutoPtr<DL::IRenderDevice>& device, const GpuResource::SharedPtr& resource);

        void DestroyImmediatly() override;
        std::any GetRawHandle() const override;
        std::vector<GpuResourceFootprint::SubresourceFootprint> GetSubresourceFootprints(const GpuResourceDescription& decription) const override;

        void* Map() override;
        void Unmap() override;

    public:
        DL::RESOURCE_DIMENSION GetResourceDimension() const { return dimension; }

        DL::ITexture* GetAsTexture() const
        {
            ASSERT(dimension != DL::RESOURCE_DIM_UNDEFINED);
            ASSERT(dimension != DL::RESOURCE_DIM_BUFFER);
            return texture;
        }

        DL::IBuffer* GetAsBuffer() const
        {
            ASSERT(dimension != DL::RESOURCE_DIM_UNDEFINED);
            ASSERT(dimension == DL::RESOURCE_DIM_BUFFER);
            return buffer;
        }

    private:
        DL::RESOURCE_DIMENSION dimension;
        union
        {
            DL::ITexture* texture;
            DL::IBuffer* buffer;
        };
    };
}