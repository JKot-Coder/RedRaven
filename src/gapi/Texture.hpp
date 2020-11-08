#pragma once

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace Render
    {

        class Texture final : public Resource
        {
        public:
            using SharedPtr = std::shared_ptr<Texture>;
            using SharedConstPtr = std::shared_ptr<const Texture>;
            using ConstSharedPtrRef = const SharedPtr&;

            enum class Type : uint32_t
            {
                Unknown,

                Texture1D,
                Texture2D,
                Texture2DMS,
                Texture3D,
                TextureCube
            };

            struct TextureDesc
            {
                static const uint32_t Maximum = -1;

                static TextureDesc Create1D(uint32_t width, Format format, uint32_t arraySize = 1, uint32_t mipLevels = Maximum)
                {
                    return TextureDesc(Type::Texture1D, width, 1, 1, format, 1, arraySize, mipLevels);
                }

                static TextureDesc Create2D(uint32_t width, uint32_t height, Format format, uint32_t arraySize = 1, uint32_t mipLevels = Maximum)
                {
                    return TextureDesc(Type::Texture2D, width, height, 1, format, 1, arraySize, mipLevels);
                }

                static TextureDesc Create2DMS(uint32_t width, uint32_t height, Format format, uint32_t sampleCount, uint32_t arraySize = 1)
                {
                    return TextureDesc(Type::Texture2DMS, width, height, 1, format, sampleCount, arraySize, 1);
                }

                static TextureDesc Create3D(uint32_t width, uint32_t height, uint32_t depth, Format format, uint32_t mipLevels = Maximum)
                {
                    return TextureDesc(Type::Texture3D, width, height, depth, format, 1, 1, mipLevels);
                }

                static TextureDesc CreateCube(uint32_t width, uint32_t height, Format format, uint32_t arraySize = 1, uint32_t mipLevels = Maximum)
                {
                    return TextureDesc(Type::TextureCube, width, height, 1, format, 1, arraySize, mipLevels);
                }

                Format format;
                Type type = Type::Unknown;
                uint32_t width = 0;
                uint32_t height = 0;
                uint32_t depth = 0;
                uint32_t mipLevels = 0;
                uint32_t sampleCount = 0;
                uint32_t arraySize = 0;

            private:
                TextureDesc(Type type, uint32_t width, uint32_t height, uint32_t depth, Format format, uint32_t sampleCount, uint32_t arraySize, uint32_t mipLevels)
                    : type(type),
                      width(width),
                      height(height),
                      depth(depth),
                      format(format),
                      sampleCount(sampleCount),
                      arraySize(arraySize),
                      mipLevels(mipLevels)
                {
                }
            };

            Texture() = delete;

            static SharedPtr Create(const TextureDesc& desc, const U8String& name, BindFlags bindFlags = BindFlags::ShaderResource)
            {
                return SharedPtr(new Texture(desc, name, bindFlags));
            }

        private:
            Texture(const TextureDesc& desc, const U8String& name, BindFlags bindFlags = BindFlags::ShaderResource);

        private:
            TextureDesc desc_;
        };

    }
}