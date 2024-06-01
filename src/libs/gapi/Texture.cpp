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

        GpuResourceViewDescription createViewDesctiption(const GpuResourceDescription& resDescription, GpuResourceFormat viewFormat, uint32_t mipLevel, uint32_t mipCount, uint32_t firstArraySlice, uint32_t arraySliceCount)
        {
            ASSERT(firstArraySlice < resDescription.texture.arraySize);
            ASSERT(mipLevel < resDescription.texture.mipLevels);

            if (mipCount == Texture::MaxPossible)
                mipCount = resDescription.texture.mipLevels - mipLevel;

            if (arraySliceCount == Texture::MaxPossible)
                arraySliceCount = resDescription.texture.arraySize - firstArraySlice;

            ASSERT(firstArraySlice + arraySliceCount <= resDescription.texture.arraySize);
            ASSERT(mipLevel + mipCount <= resDescription.texture.mipLevels);

            viewFormat = getViewFormat(resDescription.texture.format, viewFormat);
            return GpuResourceViewDescription::Texture(viewFormat, mipLevel, mipCount, firstArraySlice, arraySliceCount);
        }
    }

    ShaderResourceView::SharedPtr Texture::GetSRV(uint32_t mipLevel, uint32_t mipCount, uint32_t firstArraySlice, uint32_t numArraySlices, GpuResourceFormat format)
    {
        std::ignore = mipCount; // TODO Why?

        const auto& viewDesc = createViewDesctiption(description_, format, mipLevel, 1, firstArraySlice, numArraySlices);

        if (srvs_.find(viewDesc) == srvs_.end())
        {
            auto& deviceContext = Render::DeviceContext::Instance();
            // TODO static_pointer_cast; name_
            srvs_[viewDesc] = deviceContext.CreateShaderResourceView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc);
        }

        return srvs_[viewDesc];
    }

    DepthStencilView::SharedPtr Texture::GetDSV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices, GpuResourceFormat format)
    {
        const auto& viewDesc = createViewDesctiption(description_, format, mipLevel, 1, firstArraySlice, numArraySlices);
        // TODO VALIDATION VIEW DESC FORMAT

        if (dsvs_.find(viewDesc) == dsvs_.end())
        {
            auto& deviceContext = Render::DeviceContext::Instance();
            // TODO static_pointer_cast; name_
            dsvs_[viewDesc] = deviceContext.CreateDepthStencilView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc);
        }

        return dsvs_[viewDesc];
    }

    RenderTargetView::SharedPtr Texture::GetRTV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices, GpuResourceFormat format)
    {
        const auto& viewDesc = createViewDesctiption(description_, format, mipLevel, 1, firstArraySlice, numArraySlices);

        if (rtvs_.find(viewDesc) == rtvs_.end())
        {
            auto& deviceContext = Render::DeviceContext::Instance();
            // TODO static_pointer_cast; name_
            rtvs_[viewDesc] = deviceContext.CreateRenderTargetView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc);
        }

        return rtvs_[viewDesc];
    }

    UnorderedAccessView::SharedPtr Texture::GetUAV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices, GpuResourceFormat format)
    {
        const auto& viewDesc = createViewDesctiption(description_, format, mipLevel, 1, firstArraySlice, numArraySlices);

        if (uavs_.find(viewDesc) == uavs_.end())
        {
            auto& deviceContext = Render::DeviceContext::Instance();
            // TODO static_pointer_cast; name_
            uavs_[viewDesc] = deviceContext.CreateUnorderedAccessView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc);
        }

        return uavs_[viewDesc];
    }
}