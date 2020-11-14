#pragma once

#include "Texture.hpp"

#include "common/Math.hpp"

namespace OpenDemo
{
    namespace Render
    {

        uint32_t getMaxMipLevels(uint32_t width, uint32_t height, uint32_t depth)
        {
            ASSERT(width > 0)
            ASSERT(height > 0)
            ASSERT(depth > 0)

            const uint32_t maxDimension = std::max(width, std::max(height, depth));
            return 1 + static_cast<uint32_t>(log2(static_cast<float>(maxDimension)));
        }

        Texture::Texture(const TextureDesc& desc, const U8String& name, BindFlags bindFlags)
            : Resource(Resource::Type::Texture, name),
              desc_(desc),
              bindFlags_(bindFlags)
        {
            ASSERT(desc_.type != Type::Unknown)

            ASSERT(desc_.width > 0)
            ASSERT(desc_.height > 0)
            ASSERT(desc_.depth > 0)

            ASSERT(desc_.mipLevels > 0)
            ASSERT(desc_.sampleCount > 0)
            ASSERT(desc_.arraySize > 0)

            ASSERT(
                (desc_.sampleCount > 1 && desc_.type == Type::Texture2DMS)
                || (desc_.sampleCount == 1 && desc_.type != Type::Texture2DMS));

            switch (desc_.type)
            {
            case Type::Texture1D:
                ASSERT(desc_.height == 1)
                ASSERT(desc_.depth == 1)
                break;
            case Type::Texture2D:
            case Type::Texture2DMS:
            case Type::TextureCube:
                ASSERT(desc_.depth == 1)
                break;
            case Type::Texture3D:
                ASSERT(desc_.arraySize == 1)
                break;
            default:
                LOG_FATAL("Unsupported texture type");
            }

            if (desc_.format.IsCompressed())
            {
                ASSERT(desc_.depth == 1)
                // Size is aligned to CompressionBlock
                ASSERT(AlignTo(desc_.width, desc_.format.GetCompressionBlockWidth()) == desc_.width)
                ASSERT(AlignTo(desc_.height, desc_.format.GetCompressionBlockHeight()) == desc_.height)
            }

            // Limit/Calc maximum mips count
            desc_.mipLevels = std::min(getMaxMipLevels(desc_.width, desc_.height, desc_.depth), desc_.mipLevels);
        }

        RenderTargetView::SharedPtr Texture::GetRTV(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize)
        {
            return RenderTargetView::Create(std::dynamic_pointer_cast<Resource>(shared_from_this()), name_);
        }

    }
}