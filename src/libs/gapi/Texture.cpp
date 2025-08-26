#include "Texture.hpp"

#include "gapi/GpuResourceViews.hpp"

#include "render/DeviceContext.hpp"

#include "math/Math.hpp"
#include "common/OnScopeExit.hpp"

namespace RR::GAPI
{
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

    const ShaderResourceView* Texture::GetSRV(uint32_t mipLevel, uint32_t mipCount, uint32_t firstArraySlice, uint32_t numArraySlices, GpuResourceFormat format)
    {
        const auto& viewDesc = createViewDesc(desc_, format, mipLevel, mipCount, firstArraySlice, numArraySlices);

        if (srvs_.find(viewDesc) == srvs_.end())
        {
            // name_ !!!!!!!!!!
            srvs_[viewDesc] = Render::DeviceContext::Instance().CreateShaderResourceView(shared_from_this(), viewDesc);
        }

        return srvs_[viewDesc].get();
    }

    const DepthStencilView* Texture::GetDSV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices, GpuResourceFormat format)
    {
        const auto& viewDesc = createViewDesc(desc_, format, mipLevel, 1, firstArraySlice, numArraySlices);
        // TODO VALIDATION VIEW DESC FORMAT

        if (dsvs_.find(viewDesc) == dsvs_.end())
        {
            //  name_ !!!!!!!!
            dsvs_[viewDesc] = Render::DeviceContext::Instance().CreateDepthStencilView(eastl::static_pointer_cast<Texture>(shared_from_this()), viewDesc);
        }

        return dsvs_[viewDesc].get();
    }

    const RenderTargetView* Texture::GetRTV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices, GpuResourceFormat format)
    {
        const auto& viewDesc = createViewDesc(desc_, format, mipLevel, 1, firstArraySlice, numArraySlices);

        if (rtvs_.find(viewDesc) == rtvs_.end())
        {
            // name_ !!!!!!
           rtvs_[viewDesc] = Render::DeviceContext::Instance().CreateRenderTargetView(eastl::static_pointer_cast<Texture>(shared_from_this()), viewDesc);
        }

        return rtvs_[viewDesc].get();
    }

    const UnorderedAccessView* Texture::GetUAV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices, GpuResourceFormat format)
    {
        const auto& viewDesc = createViewDesc(desc_, format, mipLevel, 1, firstArraySlice, numArraySlices);

        if (uavs_.find(viewDesc) == uavs_.end())
        {
            //  name_ !!!!!!!!!
           uavs_[viewDesc] = Render::DeviceContext::Instance().CreateUnorderedAccessView(shared_from_this(), viewDesc);
        }

        return uavs_[viewDesc].get();
    }
}