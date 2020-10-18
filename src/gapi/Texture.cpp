#pragma once

#include "Texture.hpp"

namespace OpenDemo
{
    namespace Render
    {

        Texture::Texture(const TextureDesc& desc, const U8String& name, BindFlags bindFlags) 
            : Resource(Resource::Type::Texture, name),
              desc_(desc)
        {
            ASSERT(desc_.type != Type::Unknown)

            ASSERT(desc_.width > 0)
            ASSERT(desc_.height > 0)
            ASSERT(desc_.depth > 0)

            ASSERT(desc_.mipLevels > 0)
            ASSERT(desc_.sampleCount > 0)
            ASSERT(desc_.arraySize > 0)

            switch (desc_.type)
            {
            case Type::Texture1D:
                ASSERT(desc_.height == 1 && desc_.depth == 1 && desc_.sampleCount == 1)
                break;
            case Type::Texture2D:
                ASSERT(desc_.depth == 1 && desc_.sampleCount == 1)
                break;
            case Type::Texture2DMS:
                ASSERT(desc_.depth == 1 && desc_.sampleCount > 1)
                break;
            case Type::Texture3D:
                ASSERT(desc_.arraySize == 1 && desc_.sampleCount == 1)
                break;
            case Type::TextureCube:
                ASSERT(desc_.depth == 1 && desc_.sampleCount == 1)
                break;
            default:
                ASSERT_MSG(false, "Wrong texture type");
            }
        }
       
    }
}