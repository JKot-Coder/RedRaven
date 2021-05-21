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

            static GpuResourceDescription Buffer(uint32_t size, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource)
            {
                return GpuResourceDescription(GpuResourceDimension::Buffer, size, 1, 1, GpuResourceFormat::Unknown, bindFlags, 1, 1, 1);
            }

            static GpuResourceDescription StructuredBuffer(uint32_t size, uint32_t structSize, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource)
            {
                return GpuResourceDescription(GpuResourceDimension::Buffer, size, 1, 1, GpuResourceFormat::Unknown, bindFlags, 1, 1, 1, structSize);
            }

            static GpuResourceDescription Texture1D(uint32_t width, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture1D, width, 1, 1, format, bindFlags, 1, arraySize, mipLevels);
            }

            static GpuResourceDescription Texture2D(uint32_t width, uint32_t height, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture2D, width, height, 1, format, bindFlags, 1, arraySize, mipLevels);
            }

            static GpuResourceDescription Texture2DMS(uint32_t width, uint32_t height, GpuResourceFormat format, uint32_t sampleCount, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, uint32_t arraySize = 1)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture2DMS, width, height, 1, format, bindFlags, sampleCount, arraySize, 1);
            }

            static GpuResourceDescription Texture3D(uint32_t width, uint32_t height, uint32_t depth, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture3D, width, height, depth, format, bindFlags, 1, 1, mipLevels);
            }

            static GpuResourceDescription TextureCube(uint32_t width, uint32_t height, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::TextureCube, width, height, 1, format, bindFlags, 1, arraySize, mipLevels);
            }

            GpuResourceFormat GetFormat() const { return format_; }
            GpuResourceDimension GetDimension() const { return dimension_; }
            GpuResourceBindFlags GetBindFlags() const { return bindflags_; }

            bool IsTyped() const
            {
                return format_ != GpuResourceFormat::Unknown;
            }

            uint32_t GetSize() const
            {
                ASSERT(dimension_ == GpuResourceDimension::Buffer);
                return GetNumElements() * std::max(GetStructSize(), 1u);
            }

            uint32_t GetNumElements() const
            {
                ASSERT(dimension_ == GpuResourceDimension::Buffer);
                return width_;
            }

            uint32_t GetStructSize() const
            {
                ASSERT(dimension_ == GpuResourceDimension::Buffer);
                return structSize_;
            }

            uint32_t GetWidth(uint32_t mipLevel = 0) const
            {
                ASSERT(dimension_ != GpuResourceDimension::Buffer);
                ASSERT(mipLevel < mipLevels_);
                return (mipLevel < mipLevels_) ? std::max(1u, width_ >> mipLevel) : 0u;
            }

            uint32_t GetHeight(uint32_t mipLevel = 0) const
            {
                ASSERT(dimension_ != GpuResourceDimension::Buffer);
                ASSERT(mipLevel < mipLevels_);
                return (mipLevel < mipLevels_) ? std::max(1u, height_ >> mipLevel) : 0u;
            }

            uint32_t GetDepth(uint32_t mipLevel = 0) const
            {
                ASSERT(dimension_ != GpuResourceDimension::Buffer);
                ASSERT(mipLevel < mipLevels_);
                return (mipLevel < mipLevels_) ? std::max(1U, depth_ >> mipLevel) : 0u;
            }

            uint32_t GetSampleCount() const { return sampleCount_; }

            uint32_t GetMipCount() const
            {
                ASSERT(dimension_ != GpuResourceDimension::Buffer);
                return mipLevels_;
            }

            uint32_t GetArraySize() const
            {
                ASSERT(dimension_ != GpuResourceDimension::Buffer);
                return arraySize_;
            }

            uint32_t GetSubresourceArraySlice(uint32_t subresource) const
            {
                ASSERT(mipLevels_ > 0);
                ASSERT(subresource < GetNumSubresources());
                ASSERT(dimension_ != GpuResourceDimension::Buffer);
                const auto numFaces = (dimension_ == GpuResourceDimension::TextureCube ? 6 : 1);
                return (subresource / numFaces) / mipLevels_;
            }

            uint32_t GetSubresourceFace(uint32_t subresource) const
            {
                ASSERT(mipLevels_ > 0);
                ASSERT(subresource < GetNumSubresources());
                ASSERT(dimension_ != GpuResourceDimension::Buffer);
                return (dimension_ == GpuResourceDimension::TextureCube ? (subresource / mipLevels_) % 6 : 0);
            }

            uint32_t GetSubresourceMipLevel(uint32_t subresource) const
            {
                ASSERT(mipLevels_ > 0);
                ASSERT(subresource < GetNumSubresources());
                ASSERT(dimension_ != GpuResourceDimension::Buffer);
                return subresource % mipLevels_;
            }

            uint32_t GetSubresourceIndex(uint32_t arraySlice, uint32_t mipLevel, uint32_t faceIndex) const
            {
                ASSERT(mipLevel < mipLevels_);
                ASSERT(arraySlice < arraySize_);
                ASSERT(dimension_ != GpuResourceDimension::Buffer);

                const uint32_t numFaces = (dimension_ == GpuResourceDimension::TextureCube ? 6u : 1u);
                ASSERT(faceIndex < numFaces);

                return mipLevel + (arraySlice * numFaces + faceIndex) * mipLevels_;
            }

            uint32_t GetNumSubresources() const
            {
                constexpr uint32_t planeSlices = 1;
                const uint32_t numFaces = (dimension_ == GpuResourceDimension::TextureCube ? 6u : 1u);
                return planeSlices * numFaces * arraySize_ * mipLevels_;
            }

            uint32_t GetMaxMipLevel() const
            {
                const uint32_t maxDimension = std::max(width_, std::max(height_, depth_));
                return dimension_ == GpuResourceDimension::Buffer ? 1 : 1 + static_cast<uint32_t>(log2(static_cast<float>(maxDimension)));
            }

            bool IsValid() const;

            inline friend bool operator==(const GpuResourceDescription& lhs, const GpuResourceDescription& rhs)
            {
                return lhs.dimension_ == rhs.dimension_ &&
                       lhs.width_ == rhs.width_ &&
                       lhs.height_ == rhs.height_ &&
                       lhs.depth_ == rhs.depth_ &&
                       lhs.format_ == rhs.format_ &&
                       lhs.bindflags_ == rhs.bindflags_ &&
                       lhs.sampleCount_ == rhs.sampleCount_ &&
                       lhs.arraySize_ == rhs.arraySize_ &&
                       lhs.structSize_ == rhs.structSize_ &&
                       lhs.mipLevels_ == rhs.mipLevels_;
            }
            inline friend bool operator!=(const GpuResourceDescription& lhs, const GpuResourceDescription& rhs) { return !(lhs == rhs); }

        private:
            GpuResourceDescription(GpuResourceDimension dimension, uint32_t width, uint32_t height, uint32_t depth, GpuResourceFormat format, GpuResourceBindFlags bindFlags, uint32_t sampleCount, uint32_t arraySize, uint32_t mipLevels, uint32_t structSize = 0)
                : dimension_(dimension),
                  width_(width),
                  height_(height),
                  depth_(depth),
                  format_(format),
                  bindflags_(bindFlags),
                  sampleCount_(sampleCount),
                  arraySize_(arraySize),
                  structSize_(structSize),
                  // Limit/Calc maximum mip count
                  mipLevels_(std::min(GetMaxMipLevel(), mipLevels))
            {
                ASSERT(IsValid());
            }

        private:
            uint32_t width_;
            uint32_t height_;
            uint32_t depth_;
            uint32_t mipLevels_;
            uint32_t sampleCount_;
            uint32_t arraySize_;
            uint32_t structSize_;
            GpuResourceFormat format_;
            GpuResourceDimension dimension_;
            GpuResourceBindFlags bindflags_;

        private:
            friend class GpuResource;
            friend class Texture;
            friend class Buffer;
        };

        class CpuResourceData : public std::enable_shared_from_this<CpuResourceData>
        {
        public:
            using SharedPtr = std::shared_ptr<CpuResourceData>;
            using SharedConstPtr = std::shared_ptr<const CpuResourceData>;

            struct SubresourceFootprint
            {
                SubresourceFootprint() = default;
                SubresourceFootprint(size_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t numRows, uint32_t rowSizeInBytes, size_t rowPitch, size_t depthPitch)
                    : offset(offset), width(width), height(height), depth(depth), numRows(numRows), rowSizeInBytes(rowSizeInBytes), rowPitch(rowPitch), depthPitch(depthPitch) { }

                bool isComplatable(const SubresourceFootprint& other) const
                {
                    return (numRows == other.numRows) &&
                           (rowSizeInBytes == other.rowSizeInBytes);
                }

                size_t offset;
                uint32_t width;
                uint32_t height;
                uint32_t depth;
                uint32_t numRows;
                size_t rowSizeInBytes;
                size_t rowPitch;
                size_t depthPitch;
            };

            CpuResourceData(const std::shared_ptr<MemoryAllocation>& allocation, const GpuResourceDescription& description, const std::vector<SubresourceFootprint>& subresourceFootprints, uint32_t firstSubresource)
                : allocation_(allocation),
                  description_(description),
                  subresourceFootprints_(subresourceFootprints),
                  firstSubresource_(firstSubresource)
            {
                ASSERT(allocation);
                ASSERT(subresourceFootprints.size() > 0);
                ASSERT(firstSubresource < description.GetNumSubresources());
                ASSERT(firstSubresource + subresourceFootprints.size() <= description.GetNumSubresources());
            };

            inline std::shared_ptr<MemoryAllocation> GetAllocation() const { return allocation_; }
            inline uint32_t GetFirstSubresource() const { return firstSubresource_; }
            inline size_t GetNumSubresources() const { return subresourceFootprints_.size(); }
            inline const GpuResourceDescription& GetResourceDescription() const { return description_; }
            inline const SubresourceFootprint& GetSubresourceFootprintAt(uint32_t index) const { return subresourceFootprints_[index]; }
            inline const std::vector<SubresourceFootprint>& GetSubresourceFootprints() const { return subresourceFootprints_; }

            void CopyDataFrom(const GAPI::CpuResourceData::SharedPtr& source);

        private:
            std::shared_ptr<MemoryAllocation> allocation_;
            std::vector<SubresourceFootprint> subresourceFootprints_;
            GpuResourceDescription description_;
            uint32_t firstSubresource_;
        };

        class IGpuResource
        {
        public:
            virtual ~IGpuResource() = default;
        };

        class GpuResource : public Resource<IGpuResource>
        {
        public:
            using SharedPtr = std::shared_ptr<GpuResource>;
            using SharedConstPtr = std::shared_ptr<const GpuResource>;
            using WeakPtr = std::weak_ptr<GpuResource>;

        public:
            template <typename Type>
            std::shared_ptr<Type> GetTyped();

            inline const bool IsBuffer() const { return description_.dimension_ == GpuResourceDimension::Buffer; }
            inline const bool IsTexture() const { return description_.dimension_ != GpuResourceDimension::Buffer; }
            inline const GpuResourceDescription& GetDescription() const { return description_; }
            inline GpuResourceCpuAccess GetCpuAccess() const { return cpuAccess_; }

        protected:
            GpuResource(GpuResourceDescription description, GpuResourceCpuAccess cpuAccess, const U8String& name)
                : Resource(Object::Type::GpuResource, name),
                  description_(description),
                  cpuAccess_(cpuAccess)
            {
                ASSERT(description.IsValid());
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

        template <>
        std::shared_ptr<Buffer> GpuResource::GetTyped<Buffer>();
    }
}