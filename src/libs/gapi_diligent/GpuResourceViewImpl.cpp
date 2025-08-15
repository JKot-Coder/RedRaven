#include "GpuResourceViewImpl.hpp"

#include "gapi/GpuResource.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi_diligent/GpuResourceImpl.hpp"

#include "Utils.hpp"

#include "Texture.h"
#include "Buffer.h"
#include "TextureView.h"
#include "BufferView.h"

namespace DL = ::Diligent;

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

namespace RR::GAPI::Diligent
{
    GpuResourceViewImpl::~GpuResourceViewImpl()
    {
        // TODO Should we cast to ITextureView or IBufferView based on the resource type
        textureView->Release();
    }

    DL::TextureViewDesc getTextureViewDesc(GAPI::GpuResourceView::ViewType viewType, DL::RESOURCE_DIMENSION dimension, const GpuResourceViewDescription& desc)
    {
        auto getTextureViewType = [](GAPI::GpuResourceView::ViewType viewType) -> DL::TEXTURE_VIEW_TYPE {
            switch (viewType)
            {
                case GAPI::GpuResourceView::ViewType::ShaderResourceView:
                    return DL::TEXTURE_VIEW_SHADER_RESOURCE;
                case GAPI::GpuResourceView::ViewType::DepthStencilView:
                    return DL::TEXTURE_VIEW_DEPTH_STENCIL;
                case GAPI::GpuResourceView::ViewType::RenderTargetView:
                    return DL::TEXTURE_VIEW_RENDER_TARGET;
                case GAPI::GpuResourceView::ViewType::UnorderedAccessView:
                    return DL::TEXTURE_VIEW_UNORDERED_ACCESS;
                default:
                    ASSERT_MSG(false, "Unknown view type");
                    return DL::TEXTURE_VIEW_UNDEFINED;
            }
        };

        DL::TextureViewDesc viewDesc;
        viewDesc.ViewType = getTextureViewType(viewType);
        viewDesc.TextureDim = dimension;
        viewDesc.Format = GetDLTextureFormat(desc.format);
        viewDesc.MostDetailedMip = desc.texture.mipLevel;
        viewDesc.NumMipLevels = desc.texture.mipCount;
        viewDesc.FirstArraySlice = desc.texture.firstArraySlice;
        viewDesc.NumArraySlices = desc.texture.arraySliceCount;
        return viewDesc;
    }

    DL::ITextureView* createTextureView(
        GAPI::GpuResourceView::ViewType viewType,
        const GpuResourceViewDescription& desc,
        GpuResourceImpl* gpuResourceImpl)
    {
        DL::TextureViewDesc viewDesc = getTextureViewDesc(viewType, gpuResourceImpl->GetResourceDimension(), desc);
        DL::ITextureView* textureView = nullptr;
        gpuResourceImpl->GetAsTexture()->CreateView(viewDesc, &textureView);
        return textureView;
    }

    void GpuResourceViewImpl::Init(GAPI::GpuResourceView& view)
    {
        ASSERT(!view.GetGpuResource().expired());
        const auto& gpuResource = view.GetGpuResource().lock();
        GpuResourceImpl* gpuResourceImpl = gpuResource->GetPrivateImpl<GpuResourceImpl>();

        switch (gpuResourceImpl->GetResourceDimension())
        {
            case DL::RESOURCE_DIM_TEX_1D:
            case DL::RESOURCE_DIM_TEX_2D:
            case DL::RESOURCE_DIM_TEX_3D:
            case DL::RESOURCE_DIM_TEX_CUBE:
            case DL::RESOURCE_DIM_TEX_1D_ARRAY:
            case DL::RESOURCE_DIM_TEX_2D_ARRAY:
            case DL::RESOURCE_DIM_TEX_CUBE_ARRAY:
                textureView = createTextureView(view.GetViewType(), view.GetDescription(), gpuResourceImpl);
                break;
            case DL::RESOURCE_DIM_BUFFER:
                NOT_IMPLEMENTED();
                break;
            default:
                ASSERT_MSG(false, "Unknown resource type");
        }
    }
}
