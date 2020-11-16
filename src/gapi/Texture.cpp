#pragma once

#include "Texture.hpp"

#include "gapi/RenderContext.hpp"
#include "gapi/ResourceViews.hpp"

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
    namespace Render
    {
        Texture::Texture(const TextureDescription& desc, BindFlags bindFlags, const U8String& name)
            : Resource(Resource::ResourceType::Texture, name),
              description_(desc),
              bindFlags_(bindFlags)
        {
            ASSERT(description_.dimension != TextureDimension::Unknown)

            ASSERT(description_.width > 0)
            ASSERT(description_.height > 0)
            ASSERT(description_.depth > 0)

            ASSERT(description_.mipLevels > 0)
            ASSERT(description_.sampleCount > 0)
            ASSERT(description_.arraySize > 0)

            ASSERT(
                (description_.sampleCount > 1 && description_.dimension == TextureDimension::Texture2DMS)
                || (description_.sampleCount == 1 && description_.dimension != TextureDimension::Texture2DMS));

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

            if (description_.format.IsCompressed())
            {
                ASSERT(description_.depth == 1)
                // Size is aligned to CompressionBlock
                ASSERT(AlignTo(description_.width, description_.format.GetCompressionBlockWidth()) == description_.width)
                ASSERT(AlignTo(description_.height, description_.format.GetCompressionBlockHeight()) == description_.height)
            }

            // Limit/Calc maximum mips count
            description_.mipLevels = std::min(getMaxMipLevels(description_.width, description_.height, description_.depth), description_.mipLevels);
        }

        RenderTargetView::SharedPtr Texture::GetRTV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize)
        {
            ASSERT(firstArraySlice < description_.arraySize)
            ASSERT(IsSet(bindFlags_, BindFlags::RenderTarget))

            const ResourceViewDescription desc(mipLevel, 1, firstArraySlice, std::min(arraySize, description_.arraySize - firstArraySlice));

            auto& renderContext = RenderContext::Instance();
            return renderContext.CreateRenderTargetView(std::static_pointer_cast<Texture>(shared_from_this()), desc, name_);
        }

    }
}