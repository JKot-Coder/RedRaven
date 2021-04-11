#pragma once

#include "common/EnumClassOperators.hpp"

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/Resource.hpp"

#include <unordered_map>

namespace OpenDemo
{
    namespace GAPI
    {
        namespace
        {
            struct GpuResourceViewDescriptionHashFunc
            {
                std::size_t operator()(const GpuResourceViewDescription& desc) const
                {
                    return (std::hash<uint32_t>()(desc.texture.firstArraySlice) << 1) ^
                           (std::hash<uint32_t>()(desc.texture.arraySliceCount) << 3) ^
                           (std::hash<uint32_t>()(desc.texture.mipCount) << 5) ^
                           (std::hash<uint32_t>()(desc.texture.mipLevel) << 7);
                }
            };
        }

        enum class GpuResourceBindFlags : uint32_t
        {
            None = 0x0,
            ShaderResource = 0x01,
            UnorderedAccess = 0x02,
            RenderTarget = 0x04,
            DepthStencil = 0x08,
        };
        ENUM_CLASS_OPERATORS(GpuResourceBindFlags)

        enum class GpuResourceFormat : uint32_t
        {
            Unknown,

            RGBA32Float,
            RGBA32Uint,
            RGBA32Sint,
            RGB32Float,
            RGB32Uint,
            RGB32Sint,
            RGBA16Float,
            RGBA16Unorm,
            RGBA16Uint,
            RGBA16Snorm,
            RGBA16Sint,
            RG32Float,
            RG32Uint,
            RG32Sint,

            RGB10A2Unorm,
            RGB10A2Uint,
            R11G11B10Float,
            RGBA8Unorm,
            RGBA8UnormSrgb,
            RGBA8Uint,
            RGBA8Snorm,
            RGBA8Sint,
            RG16Float,
            RG16Unorm,
            RG16Uint,
            RG16Snorm,
            RG16Sint,

            R32Float,
            R32Uint,
            R32Sint,

            RG8Unorm,
            RG8Uint,
            RG8Snorm,
            RG8Sint,

            R16Float,
            R16Unorm,
            R16Uint,
            R16Snorm,
            R16Sint,
            R8Unorm,
            R8Uint,
            R8Snorm,
            R8Sint,
            A8Unorm,

            // Depth-stencil
            D32FloatS8X24Uint,
            D32Float,
            D24UnormS8Uint,
            D16Unorm,

            // SRV formats for depth/stencil binding
            R32FloatX8X24,
            X32G8Uint,
            R24UnormX8,
            X24G8Uint,

            // Compressed
            BC1Unorm,
            BC1UnormSrgb,
            BC2Unorm,
            BC2UnormSrgb,
            BC3Unorm,
            BC3UnormSrgb,
            BC4Unorm,
            BC4Snorm,
            BC5Unorm,
            BC5Snorm,
            BC6HU16,
            BC6HS16,
            BC7Unorm,
            BC7UnormSrgb,

            RGB16Float,
            RGB16Unorm,
            RGB16Uint,
            RGB16Snorm,
            RGB16Sint,

            RGB5A1Unorm,
            RGB9E5Float,

            BGRA8Unorm,
            BGRA8UnormSrgb,
            BGRX8Unorm,
            BGRX8UnormSrgb,

            R5G6B5Unorm,

            Count
        };

        namespace GpuResourceFormatInfo
        {
            bool IsDepth(GpuResourceFormat format);
            bool IsStencil(GpuResourceFormat format);
            bool IsCompressed(GpuResourceFormat format);

            uint32_t GetBlockSize(GpuResourceFormat format);
            uint32_t GetCompressionBlockWidth(GpuResourceFormat format);
            uint32_t GetCompressionBlockHeight(GpuResourceFormat format);

            U8String ToString(GpuResourceFormat format);
        };

        enum class GpuResourceCpuAccess : uint32_t
        {
            None,
            Read,
            Write
        };

        enum class GpuResourceDimension : uint32_t
        {
            // We can may try rid of this.
            Unknown,

            Buffer,
            Texture1D,
            Texture2D,
            Texture2DMS,
            Texture3D,
            TextureCube
        };

        struct GpuResourceDescription
        {
            static constexpr uint32_t MaxPossible = 0xFFFFFF;

            static GpuResourceDescription Create1D(uint32_t width, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture1D, width, 1, 1, format, bindFlags, 1, arraySize, mipLevels);
            }

            static GpuResourceDescription Create2D(uint32_t width, uint32_t height, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture2D, width, height, 1, format, bindFlags, 1, arraySize, mipLevels);
            }

            static GpuResourceDescription Create2DMS(uint32_t width, uint32_t height, GpuResourceFormat format, uint32_t sampleCount, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, uint32_t arraySize = 1)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture2DMS, width, height, 1, format, bindFlags, sampleCount, arraySize, 1);
            }

            static GpuResourceDescription Create3D(uint32_t width, uint32_t height, uint32_t depth, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture3D, width, height, depth, format, bindFlags, 1, 1, mipLevels);
            }

            static GpuResourceDescription CreateCube(uint32_t width, uint32_t height, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::TextureCube, width, height, 1, format, bindFlags, 1, arraySize, mipLevels);
            }

            GpuResourceFormat GetFormat() const { return format; }
            GpuResourceDimension GetDimension() const { return dimension; }
            GpuResourceBindFlags GetBindFlags() const { return bindflags; }
            uint32_t GetWidth(uint32_t mipLevel = 0) const { return (mipLevel < mipLevels) ? std::max(1U, width >> mipLevel) : 0u; }
            uint32_t GetHeight(uint32_t mipLevel = 0) const { return (mipLevel < mipLevels) ? std::max(1U, height >> mipLevel) : 0u; }
            uint32_t GetDepth(uint32_t mipLevel = 0) const { return (mipLevel < mipLevels) ? std::max(1U, depth >> mipLevel) : 0u; }
            uint32_t GetSampleCount() const { return sampleCount; }
            uint32_t GetMipCount() const { return mipLevels; }
            uint32_t GetArraySize() const { return arraySize; }

            uint32_t GetSubresourceArraySlice(uint32_t subresource) const { return subresource / mipLevels; }
            uint32_t GetSubresourceMipLevel(uint32_t subresource) const { return subresource % mipLevels; }

            uint32_t GetNumSubresources() const
            {
                constexpr uint32_t planeSlices = 1;
                const uint32_t numSets = (dimension == GpuResourceDimension::TextureCube ? 6 : 1);
                return planeSlices * numSets * arraySize * mipLevels;
            }

            uint32_t GetMaxMipLevel() const
            {
                const uint32_t maxDimension = std::max(width, std::max(height, depth));
                return 1 + static_cast<uint32_t>(log2(static_cast<float>(maxDimension)));
            }

            inline friend bool operator==(const GpuResourceDescription& lhs, const GpuResourceDescription& rhs)
            {
                return lhs.format == rhs.format &&
                       lhs.dimension == rhs.dimension &&
                       lhs.width == rhs.width &&
                       lhs.height == rhs.height &&
                       lhs.depth == rhs.depth &&
                       lhs.mipLevels == rhs.mipLevels &&
                       lhs.sampleCount == rhs.sampleCount &&
                       lhs.arraySize == rhs.arraySize;
            }
            inline friend bool operator!=(const GpuResourceDescription& lhs, const GpuResourceDescription& rhs) { return !(lhs == rhs); }

        private:
            GpuResourceDescription(GpuResourceDimension dimension, uint32_t width, uint32_t height, uint32_t depth, GpuResourceFormat format, GpuResourceBindFlags bindFlags, uint32_t sampleCount, uint32_t arraySize, uint32_t mipLevels)
                : width(width),
                  height(height),
                  depth(depth),
                  sampleCount(sampleCount),
                  arraySize(arraySize),
                  format(format),
                  bindflags(bindFlags),
                  dimension(dimension),
                  // Limit/Calc maximum mip count
                  mipLevels(std::min(GetMaxMipLevel(), mipLevels))
            {
            }

        private:
            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t depth = 0;
            uint32_t mipLevels = 0;
            uint32_t sampleCount = 0;
            uint32_t arraySize = 0;
            GpuResourceFormat format = GpuResourceFormat::Unknown;
            GpuResourceDimension dimension = GpuResourceDimension::Unknown;
            GpuResourceBindFlags bindflags = GpuResourceBindFlags::None;

        private:
            friend class GpuResource;
            friend class Texture;
            friend class Buffer;
        };

        class IGpuResource
        {
        public:
            virtual ~IGpuResource() {};
        };

        class GpuResource : public Resource<IGpuResource>
        {
        public:
            using SharedPtr = std::shared_ptr<GpuResource>;
            using SharedConstPtr = std::shared_ptr<const GpuResource>;
            using WeakPtr = std::weak_ptr<GpuResource>;

        public:
            inline GpuResourceDimension GetGpuGpuResourceDimension() const { return description_.dimension; }

            template <typename Type>
            std::shared_ptr<Type> GetTyped();

            inline const bool IsBuffer() const { return description_.dimension == GpuResourceDimension::Buffer; }
            inline const bool IsTexture() const { return description_.dimension != GpuResourceDimension::Buffer; }
            inline const GpuResourceDescription& GetDescription() const { return description_; }
            inline GpuResourceCpuAccess GetCpuAccess() const { return cpuAccess_; }

        protected:
            GpuResource(GpuResourceDescription description, GpuResourceCpuAccess cpuAccess, const U8String& name)
                : Resource(Object::Type::GpuResource, name),
                  description_(description),
                  cpuAccess_(cpuAccess)
            {
                ASSERT(description_.dimension != GpuResourceDimension::Unknown);
                ASSERT(description_.format != GpuResourceFormat::Unknown);
            };

            GpuResourceDescription description_;
            GpuResourceCpuAccess cpuAccess_;

            std::unordered_map<GpuResourceViewDescription, std::shared_ptr<ShaderResourceView>, GpuResourceViewDescriptionHashFunc> srvs_;
            std::unordered_map<GpuResourceViewDescription, std::shared_ptr<RenderTargetView>, GpuResourceViewDescriptionHashFunc> rtvs_;
            std::unordered_map<GpuResourceViewDescription, std::shared_ptr<DepthStencilView>, GpuResourceViewDescriptionHashFunc> dsvs_;
            std::unordered_map<GpuResourceViewDescription, std::shared_ptr<UnorderedAccessView>, GpuResourceViewDescriptionHashFunc> uavs_;
        };

        template <>
        std::shared_ptr<Texture> GpuResource::GetTyped<Texture>();
        /*
        template <>
        Buffer& GpuResource::GetTyped<Buffer>();*/
    }
}