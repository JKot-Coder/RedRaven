#pragma once

#include "gapi/GpuResource.hpp"

namespace RR::GAPI
{
    struct TextureDescription
    {
    public:
        static constexpr uint32_t MaxPossible = 0xFFFFFF;

        enum class Dimension : uint32_t
        {
            Texture1D,
            Texture2D,
            Texture2DMS,
            Texture3D,
            TextureCube
        };

    public:
        static TextureDescription Texture1D(uint32_t width, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
        {
            return TextureDescription(Dimension::Texture1D, width, 1, 1, format, bindFlags, usage, MultisampleType::None, arraySize, mipLevels);
        }

        static TextureDescription Texture2D(uint32_t width, uint32_t height, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
        {
            return TextureDescription(Dimension::Texture2D, width, height, 1, format, bindFlags, usage, MultisampleType::None, arraySize, mipLevels);
        }

        static TextureDescription Texture2DMS(uint32_t width, uint32_t height, GpuResourceFormat format, MultisampleType multisampleType, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default, uint32_t arraySize = 1)
        {
            return TextureDescription(Dimension::Texture2DMS, width, height, 1, format, bindFlags, usage, multisampleType, arraySize, 1);
        }

        static TextureDescription Texture3D(uint32_t width, uint32_t height, uint32_t depth, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default, uint32_t mipLevels = MaxPossible)
        {
            return TextureDescription(Dimension::Texture3D, width, height, depth, format, bindFlags, usage, MultisampleType::None, 1, mipLevels);
        }

        static TextureDescription TextureCube(uint32_t width, uint32_t height, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
        {
            return TextureDescription(Dimension::TextureCube, width, height, 1, format, bindFlags, usage, MultisampleType::None, arraySize, mipLevels);
        }

        inline friend bool operator==(const TextureDescription& lhs, const TextureDescription& rhs)
        {
            static_assert(sizeof(TextureDescription) == 10 * sizeof(uint32_t), "Check for tighly packed structure");
            return std::memcmp(&lhs, &rhs, sizeof(TextureDescription)) == 0;
        }

        inline friend bool operator!=(const TextureDescription& lhs, const TextureDescription& rhs) { return !(lhs == rhs); }

    public:
        TextureDescription() = default;

        uint32_t GetWidth(uint32_t mipLevel = 0) const
        {
            ASSERT(mipLevel < mipLevels);
            return (mipLevel < mipLevels) ? std::max(1u, width >> mipLevel) : 0u;
        }

        uint32_t GetHeight(uint32_t mipLevel = 0) const
        {
            ASSERT(mipLevel < mipLevels);
            return (mipLevel < mipLevels) ? std::max(1u, height >> mipLevel) : 0u;
        }

        uint32_t GetDepth(uint32_t mipLevel = 0) const
        {
            ASSERT(mipLevel < mipLevels);
            return (mipLevel < mipLevels) ? std::max(1U, depth >> mipLevel) : 0u;
        }

        uint32_t GetSubresourceArraySlice(uint32_t subresource) const
        {
            ASSERT(mipLevels > 0);
            ASSERT(subresource < GetNumSubresources());
            const auto numFaces = (dimension == Dimension::TextureCube ? 6 : 1);
            return (subresource / numFaces) / mipLevels;
        }

        uint32_t GetSubresourceFace(uint32_t subresource) const
        {
            ASSERT(mipLevels > 0);
            ASSERT(subresource < GetNumSubresources());
            return (dimension == Dimension::TextureCube ? (subresource / mipLevels) % 6 : 0);
        }

        uint32_t GetSubresourceMipLevel(uint32_t subresource) const
        {
            ASSERT(mipLevels > 0);
            ASSERT(subresource < GetNumSubresources());
            return subresource % mipLevels;
        }

        uint32_t GetSubresourceIndex(uint32_t arraySlice, uint32_t mipLevel, uint32_t faceIndex) const
        {
            ASSERT(mipLevel < mipLevels);
            ASSERT(arraySlice < arraySize);

            const uint32_t numFaces = (dimension == Dimension::TextureCube ? 6u : 1u);
            ASSERT(faceIndex < numFaces);

            return mipLevel + (arraySlice * numFaces + faceIndex) * mipLevels;
        }

        uint32_t GetNumSubresources() const
        {
            constexpr uint32_t planeSlices = 1;
            const uint32_t numFaces = (dimension == Dimension::TextureCube ? 6u : 1u);
            return planeSlices * numFaces * arraySize * mipLevels;
        }

        uint32_t GetMaxMipLevel() const
        {
            const uint32_t maxDimension = std::max(width, std::max(height, depth));
            return 1 + static_cast<uint32_t>(log2(static_cast<float>(maxDimension)));
        }

        bool IsValid() const;

    private:
        TextureDescription(Dimension dimension, uint32_t width, uint32_t height, uint32_t depth, GpuResourceFormat format, GpuResourceBindFlags bindFlags, GpuResourceUsage usage, MultisampleType multisampleType, uint32_t arraySize, uint32_t mipLevels)
            : dimension(dimension),
              width(width),
              height(height),
              depth(depth),
              format(format),
              bindFlags(bindFlags),
              usage(usage),
              multisampleType(multisampleType),
              arraySize(arraySize),
              mipLevels(mipLevels)
        {
            // Limit/Calc maximum mip count
            this->mipLevels = (std::min(GetMaxMipLevel(), mipLevels));
            ASSERT(IsValid());
        }

    public:
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        uint32_t arraySize = 1;
        MultisampleType multisampleType = MultisampleType::None;
        GpuResourceFormat format = GpuResourceFormat::Unknown;
        Dimension dimension = Dimension::Texture2D;
        GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource;
        GpuResourceUsage usage = GpuResourceUsage::Default;
    };

    class Texture final : public GpuResource
    {
    public:
        using SharedPtr = std::shared_ptr<Texture>;
        using SharedConstPtr = std::shared_ptr<const Texture>;

        static constexpr uint32_t MaxPossible = 0xFFFFFF;

    public:
        std::shared_ptr<ShaderResourceView> GetSRV(uint32_t mipLevel = 0, uint32_t mipCount = MaxPossible, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        std::shared_ptr<RenderTargetView> GetRTV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        std::shared_ptr<DepthStencilView> GetDSV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        std::shared_ptr<UnorderedAccessView> GetUAV(uint32_t mipLevel, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);

    private:
        static SharedPtr Create(
            const GpuResourceDescription& description,
            IDataBuffer::SharedPtr initialData,
            const U8String& name)
        {
            return SharedPtr(new Texture(description, initialData, name));
        }

        Texture(const GpuResourceDescription& description, IDataBuffer::SharedPtr initialData, const U8String& name)
            : GpuResource(description, initialData, name)
        {
            if (!description.IsTexture())            
                LOG_FATAL("Wrong Description");
        };

    private:
        friend class Render::DeviceContext;
    };
}