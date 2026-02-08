#include "RenderTarget.hpp"

#include "gapi/Texture.hpp"

#include "render/DeviceContext.hpp"

namespace RR::Render
{
    using namespace RR::GAPI;

    namespace
    {
        GpuResourceFormat getViewFormat(GpuResourceFormat resourceFormat, GpuResourceFormat viewFormat)
        {
            if (viewFormat == GpuResourceFormat::Unknown)
            {
                const bool isDepthStencil = GpuResourceFormatInfo::IsDepth(resourceFormat) && GpuResourceFormatInfo::IsStencil(resourceFormat);
                UNUSED(isDepthStencil);
                ASSERT(!isDepthStencil);

                return resourceFormat;
            }
            // TODO VALIDATION
            return viewFormat;
        }

        GpuResourceViewDesc createViewDesc(const GpuResourceDesc& resDesc, GpuResourceFormat viewFormat, uint32_t mipLevel, uint32_t mipCount, uint32_t firstArraySlice, uint32_t arraySliceCount)
        {
            ASSERT(firstArraySlice < resDesc.texture.arraySize);
            ASSERT(mipLevel < resDesc.texture.mipLevels);

            if (mipCount == Texture::MaxPossible)
                mipCount = resDesc.texture.mipLevels - mipLevel;

            if (arraySliceCount == Texture::MaxPossible)
                arraySliceCount = resDesc.texture.arraySize - firstArraySlice;

            ASSERT(firstArraySlice + arraySliceCount <= resDesc.texture.arraySize);
            ASSERT(mipLevel + mipCount <= resDesc.texture.mipLevels);

            viewFormat = getViewFormat(resDesc.texture.format, viewFormat);
            return GpuResourceViewDesc::Texture(viewFormat, mipLevel, mipCount, firstArraySlice, arraySliceCount);
        }
    }

    const GAPI::ShaderResourceView* RenderTarget::GetSRV(uint32_t mipLevel, uint32_t mipCount, uint32_t firstArraySlice, uint32_t numArraySlices, GAPI::GpuResourceFormat format)
    {
        const auto& desc = texture_->GetDesc();
        const auto& viewDesc = createViewDesc(desc, format, mipLevel, mipCount, firstArraySlice, numArraySlices);

        if (srvs_.find(viewDesc) == srvs_.end())
        {
            // name_ !!!!!!!!!!
            srvs_[viewDesc] = Render::DeviceContext::Instance().CreateShaderResourceView(*texture_, viewDesc);
        }

        return srvs_[viewDesc].get();
    }

    const GAPI::DepthStencilView* RenderTarget::GetDSV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices, GAPI::GpuResourceFormat format)
    {
        const auto& desc = texture_->GetDesc();
        const auto& viewDesc = createViewDesc(desc, format, mipLevel, 1, firstArraySlice, numArraySlices);
        // TODO VALIDATION VIEW DESC FORMAT

        if (dsvs_.find(viewDesc) == dsvs_.end())
        {
            //  name_ !!!!!!!!
            dsvs_[viewDesc] = Render::DeviceContext::Instance().CreateDepthStencilView(*texture_, viewDesc);
        }

        return dsvs_[viewDesc].get();
    }

    const GAPI::RenderTargetView* RenderTarget::GetRTV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices, GAPI::GpuResourceFormat format)
    {
        const auto& desc = texture_->GetDesc();
        const auto& viewDesc = createViewDesc(desc, format, mipLevel, 1, firstArraySlice, numArraySlices);

        if (rtvs_.find(viewDesc) == rtvs_.end())
        {
            // name_ !!!!!!
           rtvs_[viewDesc] = Render::DeviceContext::Instance().CreateRenderTargetView(*texture_, viewDesc);
        }

        return rtvs_[viewDesc].get();
    }

    const GAPI::UnorderedAccessView* RenderTarget::GetUAV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices, GAPI::GpuResourceFormat format)
    {
        const auto& desc = texture_->GetDesc();
        const auto& viewDesc = createViewDesc(desc, format, mipLevel, 1, firstArraySlice, numArraySlices);

        if (uavs_.find(viewDesc) == uavs_.end())
        {
            //  name_ !!!!!!!!!
           uavs_[viewDesc] = Render::DeviceContext::Instance().CreateUnorderedAccessView(*texture_, viewDesc);
        }

        return uavs_[viewDesc].get();
    }
}