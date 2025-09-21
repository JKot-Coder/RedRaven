#pragma once

#include "gapi/GpuResource.hpp"

#include "RefCntAutoPtr.hpp"
#include "GraphicsTypes.h"

namespace DL = ::Diligent;
namespace Diligent
{
    struct IRenderDevice;
    struct ITexture;
    struct IBuffer;
}

namespace RR::GAPI::Diligent
{
    class GpuResourceImpl final : public RR::GAPI::IGpuResource
    {
    public:
        GpuResourceImpl() = default;
        ~GpuResourceImpl() override;

        void Init(const DL::RefCntAutoPtr<DL::IRenderDevice>& device, const GpuResource& resource);
        void Init(DL::ITexture* texture, const GpuResource& resource);

        void DestroyImmediatly() override;
        std::any GetRawHandle() const override;
        std::vector<GpuResourceFootprint::SubresourceFootprint> GetSubresourceFootprints(const GpuResourceDesc& desc) const override;

        void* Map() override;
        void Unmap() override;
        void DestroyResource();

    public:
        DL::RESOURCE_DIMENSION GetResourceDimension() const { return dimension; }

        DL::ITexture* GetAsTexture() const
        {
            ASSERT(dimension != DL::RESOURCE_DIM_UNDEFINED);
            ASSERT(dimension != DL::RESOURCE_DIM_BUFFER);
            return texture_;
        }

        DL::IBuffer* GetAsBuffer() const
        {
            ASSERT(dimension != DL::RESOURCE_DIM_UNDEFINED);
            ASSERT(dimension == DL::RESOURCE_DIM_BUFFER);
            return buffer_;
        }

    private:
        DL::RESOURCE_DIMENSION dimension;
        union
        {
            DL::ITexture* texture_;
            DL::IBuffer* buffer_;
        };
    };
}