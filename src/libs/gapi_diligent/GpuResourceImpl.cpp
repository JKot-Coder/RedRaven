#include "GpuResourceImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "Utils.hpp"

#include "RenderDevice.h"
#include "Texture.h"
#include "Buffer.h"

namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    GpuResourceImpl::~GpuResourceImpl()
    {
        ASSERT(dimension != DL::RESOURCE_DIM_UNDEFINED);
        DestroyResource();
    }

    void GpuResourceImpl::DestroyResource()
    {
        if (!buffer_)
            return;

        if (dimension == DL::RESOURCE_DIM_BUFFER)
            buffer_->Release();
        else
            texture_->Release();

        buffer_ = nullptr;
    }

    DL::RESOURCE_DIMENSION getResourceDimension(const GpuResourceDimension& dimension)
    {
        switch (dimension)
        {
            case GpuResourceDimension::Texture1D:
                return DL::RESOURCE_DIM_TEX_1D;
            case GpuResourceDimension::Texture2D:
                return DL::RESOURCE_DIM_TEX_2D;
            case GpuResourceDimension::Texture3D:
                return DL::RESOURCE_DIM_TEX_3D;
            default:
                ASSERT_MSG(false, "Unknown dimension");
                return DL::RESOURCE_DIM_UNDEFINED;
        }
    }

    DL::USAGE getUsage(const GpuResourceUsage& usage)
    {
        switch (usage)
        {
            case GpuResourceUsage::Default:
                return DL::USAGE_DEFAULT;
            default:
                ASSERT_MSG(false, "Unknown usage");
                return DL::USAGE_DEFAULT;
        }
    }

    DL::BIND_FLAGS getBindFlags(const GpuResourceBindFlags& bindFlags)
    {
        DL::BIND_FLAGS flags = DL::BIND_NONE;

        if (IsSet(bindFlags, GpuResourceBindFlags::ShaderResource))
            flags |= DL::BIND_SHADER_RESOURCE;
        if (IsSet(bindFlags, GpuResourceBindFlags::UnorderedAccess))
            flags |= DL::BIND_UNORDERED_ACCESS;
        if (IsSet(bindFlags, GpuResourceBindFlags::RenderTarget))
            flags |= DL::BIND_RENDER_TARGET;
        if (IsSet(bindFlags, GpuResourceBindFlags::DepthStencil))
            flags |= DL::BIND_DEPTH_STENCIL;

        static_assert(static_cast<uint32_t>(GpuResourceBindFlags::Count) == 4);
        return flags;
    }

    DL::TextureDesc getTextureDesc(const GpuResourceDesc& desc, const std::string& name)
    {
        DL::TextureDesc texDesc;
        texDesc.Name = name.c_str();
        texDesc.Format = GetDLTextureFormat(desc.texture.format);
        texDesc.Type = getResourceDimension(desc.dimension);
        texDesc.Width = desc.texture.width;
        texDesc.Height = desc.texture.height;
        texDesc.Depth = desc.texture.depth;
        texDesc.MipLevels = desc.texture.mipLevels;
        texDesc.ArraySize = desc.texture.arraySize;
        texDesc.Usage = getUsage(desc.usage);
        texDesc.BindFlags = getBindFlags(desc.bindFlags);
        return texDesc;
    }

    DL::RESOURCE_DIMENSION getDLResourceDimension(GAPI::GpuResourceDimension dimension)
    {
        switch (dimension)
        {
            case GAPI::GpuResourceDimension::Buffer:
                return DL::RESOURCE_DIM_BUFFER;
            case GAPI::GpuResourceDimension::Texture1D:
                return DL::RESOURCE_DIM_TEX_1D;
            case GAPI::GpuResourceDimension::Texture2D:
                return DL::RESOURCE_DIM_TEX_2D;
            case GAPI::GpuResourceDimension::Texture3D:
                return DL::RESOURCE_DIM_TEX_3D;
            case GAPI::GpuResourceDimension::TextureCube:
                return DL::RESOURCE_DIM_TEX_CUBE;
            default:
                ASSERT_MSG(false, "Unknown resource dimension");
                return DL::RESOURCE_DIM_UNDEFINED;
        }
    }

    void GpuResourceImpl::Init(const DL::RefCntAutoPtr<DL::IRenderDevice>& device, const GpuResource& resource)
    {
        ASSERT_MSG(!resource.GetInitialData(), "Initial data isn't supported");

        const auto desc = getTextureDesc(resource.GetDesc(), resource.GetName());
        device->CreateTexture(desc, nullptr, &texture_);
        dimension = getDLResourceDimension(resource.GetDesc().GetDimension());
    }

    void GpuResourceImpl::Init(DL::ITexture* texture, const GpuResource& resource)
    {
        ASSERT(texture);
        ASSERT(!texture_);

        texture_ = texture;
        dimension = getDLResourceDimension(resource.GetDesc().GetDimension());
    }

    void GpuResourceImpl::DestroyImmediatly() { NOT_IMPLEMENTED(); }

    std::any GpuResourceImpl::GetRawHandle() const
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    std::vector<GpuResourceFootprint::SubresourceFootprint> GpuResourceImpl::GetSubresourceFootprints(const GpuResourceDesc& desc) const
    {
        UNUSED(desc);
        NOT_IMPLEMENTED();
        return {};
    }

    void* GpuResourceImpl::Map()
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }
    void GpuResourceImpl::Unmap() { NOT_IMPLEMENTED(); }


}