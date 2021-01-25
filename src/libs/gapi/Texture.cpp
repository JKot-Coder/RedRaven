#pragma once

#include "Texture.hpp"

#include "gapi/GpuResourceViews.hpp"

#include "render/RenderContext.hpp"

#include "common/Math.hpp"

namespace
{
    uint32_t getMaxMipLevels(uint32_t width, uint32_t height, uint32_t depth)
    {
        ASSERT(width > 0)
        ASSERT(height > 0)
        ASSERT(depth > 0)

        const uint32_t maxDimension = std::max(width, std::max(height, depth));
        return 1 + static_cast<uint32_t>(log2(static_cast<float>(maxDimension)));
    }
}

namespace OpenDemo
{
    namespace GAPI
    {
        namespace
        {
            GpuResourceViewDescription createViewDesctiption(const TextureDescription& resDesctiption, uint32_t mipLevel, uint32_t mipCount, uint32_t firstArraySlice, uint32_t arraySliceCount)
            {
                const auto resArraySize = resDesctiption.arraySize;
                const auto resMipLevels = resDesctiption.mipLevels;

                ASSERT(firstArraySlice < resArraySize);
                ASSERT(mipLevel < resMipLevels);

                if (mipCount == GpuResourceViewDescription::MaxPossible)
                    mipCount = resMipLevels - mipLevel;

                if (arraySliceCount == GpuResourceViewDescription::MaxPossible)
                    arraySliceCount = resArraySize - firstArraySlice;

                ASSERT(firstArraySlice + arraySliceCount <= resArraySize);
                ASSERT(mipLevel + mipCount <= resMipLevels);

                return GpuResourceViewDescription(mipLevel, mipCount, firstArraySlice, arraySliceCount);
            }
        }

        Texture::Texture(const TextureDescription& desc, GpuResourceBindFlags bindFlags, const U8String& name)
            : GpuResource(GpuResource::Type::Texture, bindFlags, name),
              description_(desc)
        {
            ASSERT(description_.format != GpuResourceFormat::Unknown)
            ASSERT(description_.dimension != TextureDimension::Unknown)

            ASSERT((description_.sampleCount > 1 && description_.dimension == TextureDimension::Texture2DMS) ||
                   (description_.sampleCount == 1 && description_.dimension != TextureDimension::Texture2DMS));

            switch (description_.dimension)
            {
            case TextureDimension::Texture1D:
                ASSERT(description_.height == 1)
                ASSERT(description_.depth == 1)
                break;
            case TextureDimension::Texture2D:
            case TextureDimension::Texture2DMS:
            case TextureDimension::TextureCube:
                ASSERT(description_.depth == 1)
                break;
            case TextureDimension::Texture3D:
                ASSERT(description_.arraySize == 1)
                break;
            default:
                LOG_FATAL("Unsupported texture type");
            }

            if (GpuResourceFormatInfo::IsCompressed(description_.format))
            {
                ASSERT(description_.depth == 1)
                // Size is aligned to CompressionBlock
                ASSERT(AlignTo(description_.width, GpuResourceFormatInfo::GetCompressionBlockWidth(description_.format)) == description_.width)
                ASSERT(AlignTo(description_.height, GpuResourceFormatInfo::GetCompressionBlockHeight(description_.format)) == description_.height)
            }

            // Limit/Calc maximum mip count
            description_.mipLevels = std::min(getMaxMipLevels(description_.width, description_.height, description_.depth), description_.mipLevels);
        }

        ShaderResourceView::SharedPtr Texture::GetSRV(uint32_t mipLevel, uint32_t mipCount, uint32_t firstArraySlice, uint32_t numArraySlices)
        {
            const auto& viewDesc = createViewDesctiption(description_, mipLevel, 1, firstArraySlice, numArraySlices);

            if (srvs_.find(viewDesc) == srvs_.end())
            {
                auto& renderContext = Render::RenderContext::Instance();
                // TODO static_pointer_cast; name_
                srvs_[viewDesc] = renderContext.CreateShaderResourceView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc, name_);
            }

            return srvs_[viewDesc];
        }

        DepthStencilView::SharedPtr Texture::GetDSV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices)
        {
            const auto& viewDesc = createViewDesctiption(description_, mipLevel, 1, firstArraySlice, numArraySlices);

            if (dsvs_.find(viewDesc) == dsvs_.end())
            {
                auto& renderContext = Render::RenderContext::Instance();
                // TODO static_pointer_cast; name_
                dsvs_[viewDesc] = renderContext.CreateDepthStencilView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc, name_);
            }

            return dsvs_[viewDesc];
        }

        RenderTargetView::SharedPtr Texture::GetRTV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices)
        {
            const auto& viewDesc = createViewDesctiption(description_, mipLevel, 1, firstArraySlice, numArraySlices);

            if (rtvs_.find(viewDesc) == rtvs_.end())
            {
                auto& renderContext = Render::RenderContext::Instance();
                // TODO static_pointer_cast; name_
                rtvs_[viewDesc] = renderContext.CreateRenderTargetView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc, name_);
            }

            return rtvs_[viewDesc];
        }

        UnorderedAccessView::SharedPtr Texture::GetUAV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices)
        {
            const auto& viewDesc = createViewDesctiption(description_, mipLevel, 1, firstArraySlice, numArraySlices);

            if (uavs_.find(viewDesc) == uavs_.end())
            {
                auto& renderContext = Render::RenderContext::Instance();
                // TODO static_pointer_cast; name_
                uavs_[viewDesc] = renderContext.CreateUnorderedAccessView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc, name_);
            }

            return uavs_[viewDesc];
        }
    }
}