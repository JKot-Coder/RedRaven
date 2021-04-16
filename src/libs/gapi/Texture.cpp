#pragma once

#include "Texture.hpp"

#include "gapi/GpuResourceViews.hpp"
#include "gapi/MemoryAllocation.hpp"

#include "render/DeviceContext.hpp"

#include "common/Math.hpp"
#include "common/OnScopeExit.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace
        {
            GpuResourceViewDescription createViewDesctiption(const GpuResourceDescription& resDesctiption, uint32_t mipLevel, uint32_t mipCount, uint32_t firstArraySlice, uint32_t arraySliceCount)
            {
                const auto resArraySize = resDesctiption.GetArraySize();
                const auto resMipLevels = resDesctiption.GetMipCount();

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

        Texture::Texture(const GpuResourceDescription& description, GpuResourceCpuAccess cpuAccess, const U8String& name)
            : GpuResource(description, cpuAccess, name)
        {
            ASSERT(description_.dimension != GpuResourceDimension::Unknown);
            ASSERT(description_.format != GpuResourceFormat::Unknown);

            ASSERT((description_.sampleCount > 1 && description_.dimension == GpuResourceDimension::Texture2DMS) ||
                   (description_.sampleCount == 1 && description_.dimension != GpuResourceDimension::Texture2DMS));

            switch (description_.dimension)
            {
            case GpuResourceDimension::Texture1D:
                ASSERT(description_.height == 1);
                ASSERT(description_.depth == 1);
                break;
            case GpuResourceDimension::Texture2D:
            case GpuResourceDimension::TextureCube:
                ASSERT(description_.depth == 1);
                break;
            case GpuResourceDimension::Texture2DMS:
                ASSERT(description_.depth == 1);
                ASSERT(IsSet(description_.bindflags, GpuResourceBindFlags::RenderTarget));
                break;
            case GpuResourceDimension::Texture3D:
                ASSERT(description_.arraySize == 1);
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

            ASSERT(description_.mipLevels <= description_.GetMaxMipLevel());
        }

        ShaderResourceView::SharedPtr Texture::GetSRV(uint32_t mipLevel, uint32_t mipCount, uint32_t firstArraySlice, uint32_t numArraySlices)
        {
            const auto& viewDesc = createViewDesctiption(description_, mipLevel, 1, firstArraySlice, numArraySlices);

            if (srvs_.find(viewDesc) == srvs_.end())
            {
                auto& renderContext = Render::RenderContext::Instance();
                // TODO static_pointer_cast; name_
                srvs_[viewDesc] = renderContext.CreateShaderResourceView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc);
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
                dsvs_[viewDesc] = renderContext.CreateDepthStencilView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc);
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
                rtvs_[viewDesc] = renderContext.CreateRenderTargetView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc);
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
                uavs_[viewDesc] = renderContext.CreateUnorderedAccessView(std::static_pointer_cast<Texture>(shared_from_this()), viewDesc);
            }

            return uavs_[viewDesc];
        }

        void CpuResourceData::CopyDataFrom(const GAPI::CpuResourceData::SharedPtr& source)
        {
            ASSERT(source);
            ASSERT(source != shared_from_this());
            static_assert(static_cast<int>(MemoryAllocationType::Count) == 3);
            ASSERT(allocation_->GetMemoryType() != GAPI::MemoryAllocationType::Readback);
            ASSERT(source->GetAllocation()->GetMemoryType() != GAPI::MemoryAllocationType::Upload);
            ASSERT(source->GetFirstSubresource() == GetFirstSubresource());
            ASSERT(source->GetNumSubresources() == GetNumSubresources());

            const auto sourceDataPointer = static_cast<uint8_t*>(source->GetAllocation()->Map());
            const auto destDataPointer = static_cast<uint8_t*>(allocation_->Map());

            ON_SCOPE_EXIT(
                {
                    source->GetAllocation()->Unmap();
                    allocation_->Unmap();
                });

            const auto numSubresources = source->GetNumSubresources();
            for (uint32_t index = 0; index < numSubresources; index++)
            {
                const auto& sourceFootprint = source->GetSubresourceFootprintAt(index);
                const auto& destFootprint = GetSubresourceFootprintAt(index);

                ASSERT(sourceFootprint.isComplatable(destFootprint));

                for (uint32_t depth = 0; depth < sourceFootprint.depth; depth++)
                {
                    auto sourceRowPointer = sourceDataPointer + sourceFootprint.offset + sourceFootprint.depthPitch * depth;
                    auto destRowPointer = destDataPointer + destFootprint.offset + destFootprint.depthPitch * depth;

                    for (uint32_t row = 0; row < sourceFootprint.numRows; row++)
                    {
                        std::memcpy(destRowPointer, sourceRowPointer, sourceFootprint.rowSizeInBytes);

                        sourceRowPointer += sourceFootprint.rowPitch;
                        destRowPointer += destFootprint.rowPitch;
                    }
                }
            }
        }
    }
}