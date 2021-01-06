#pragma once

#include "gapi/GpuResource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        enum class TextureDimension : uint32_t
        {
            Unknown,

            Texture1D,
            Texture2D,
            Texture2DMS,
            Texture3D,
            TextureCube
        };

        struct TextureDescription
        {
            static constexpr uint32_t MaxPossible = 0xFFFFFF;

            static TextureDescription Create1D(uint32_t width, GpuResourceFormat format, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return TextureDescription(TextureDimension::Texture1D, width, 1, 1, format, 1, arraySize, mipLevels);
            }

            static TextureDescription Create2D(uint32_t width, uint32_t height, GpuResourceFormat format, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return TextureDescription(TextureDimension::Texture2D, width, height, 1, format, 1, arraySize, mipLevels);
            }

            static TextureDescription Create2DMS(uint32_t width, uint32_t height, GpuResourceFormat format, uint32_t sampleCount, uint32_t arraySize = 1)
            {
                return TextureDescription(TextureDimension::Texture2DMS, width, height, 1, format, sampleCount, arraySize, 1);
            }

            static TextureDescription Create3D(uint32_t width, uint32_t height, uint32_t depth, GpuResourceFormat format, uint32_t mipLevels = MaxPossible)
            {
                return TextureDescription(TextureDimension::Texture3D, width, height, depth, format, 1, 1, mipLevels);
            }

            static TextureDescription CreateCube(uint32_t width, uint32_t height, GpuResourceFormat format, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return TextureDescription(TextureDimension::TextureCube, width, height, 1, format, 1, arraySize, mipLevels);
            }

            GpuResourceFormat format;
            TextureDimension dimension = TextureDimension::Unknown;
            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t depth = 0;
            uint32_t mipLevels = 0;
            uint32_t sampleCount = 0;
            uint32_t arraySize = 0;

        private:
            TextureDescription(TextureDimension dimension, uint32_t width, uint32_t height, uint32_t depth, GpuResourceFormat format, uint32_t sampleCount, uint32_t arraySize, uint32_t mipLevels)
                : dimension(dimension),
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

        class Texture final : public GpuResource
        {
        public:
            using SharedPtr = std::shared_ptr<Texture>;
            using SharedConstPtr = std::shared_ptr<const Texture>;

            static constexpr uint32_t MaxPossible = 0xFFFFFF;

        public:
            //      ShaderResourceView::SharedPtr getSRV(uint32_t mostDetailedMip, uint32_t mipCount = kMaxPossible, uint32_t firstArraySlice = 0, uint32_t arraySize = kMaxPossible);

            std::shared_ptr<RenderTargetView> GetRTV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible);

            // DepthStencilView::SharedPtr getDSV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t arraySize = kMaxPossible);

            //   UnorderedAccessView::SharedPtr getUAV(uint32_t mipLevel, uint32_t firstArraySlice = 0, uint32_t arraySize = kMaxPossible);

            const TextureDescription& GetDescription() const { return description_; }
            GpuResourceBindFlags GetBindFlags() const { return bindFlags_; }

        private:
            static SharedPtr Create(const TextureDescription& description, GpuResourceBindFlags bindFlags, const U8String& name)
            {
                return SharedPtr(new Texture(description, bindFlags, name));
            }

            Texture(const TextureDescription& description, GpuResourceBindFlags bindFlags, const U8String& name);

        private:
            TextureDescription description_;
            GpuResourceBindFlags bindFlags_;

            // TODO remove this
            std::shared_ptr<RenderTargetView> rtv = nullptr;

            friend class Render::RenderContext;
        };
    }
}