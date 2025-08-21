#pragma once

#include "common/DataBuffer.hpp"
#include "common/EnumClassOperators.hpp"

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/Resource.hpp"

#include <unordered_map>

// TODO Temporary
#include <any>

namespace RR::Render
{
    class DeviceContext;
}

namespace RR
{
    using namespace Common;

    namespace GAPI
    {
        enum class GpuResourceBindFlags : uint32_t
        {
            None = 0,
            ShaderResource = 1 << 0,
            UnorderedAccess = 1 << 1,
            RenderTarget = 1 << 2,
            DepthStencil = 1 << 3,
            Count = 4,
        };
        ENUM_CLASS_BITWISE_OPS(GpuResourceBindFlags)

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

            RGB16Float, // Unsuported on most GAPI
            RGB16Unorm, // Unsuported on most GAPI
            RGB16Uint, // Unsuported on most GAPI
            RGB16Snorm, // Unsuported on most GAPI
            RGB16Sint, // Unsuported on most GAPI

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

            std::string ToString(GpuResourceFormat format);
        };

        enum class MultisampleType : uint32_t
        {
            None,
            MSAA_2
        };

        enum class BufferFlags : uint32_t
        {
            None = 0,
            RawBuffer = 1 << 0,
            StructuredBuffer = 1 << 1,
            IndexBuffer = 1 << 2,
        };
        ENUM_CLASS_BITWISE_OPS(BufferFlags)

        enum class GpuResourceUsage : uint32_t
        {
            Default, // CPU no access, GPU read/write
            Upload, // CPU write, GPU read
            Readback // CPU read, GPU write
        };

        enum class GpuResourceDimension : uint32_t
        {
            Buffer,
            Texture1D,
            Texture2D,
            Texture2DMS,
            Texture3D,
            TextureCube,
            Count
        };

        // TODO: make non copyable use pass by value and move tehnique
        struct GpuResourceDescription
        {
        public:
            static constexpr uint32_t MaxPossible = 0xFFFFFF;

            static GpuResourceDescription Buffer(size_t size, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default)
            {
                return GpuResourceDescription(size, 0, BufferFlags::None, bindFlags, usage);
            }

            static GpuResourceDescription StructuredBuffer(size_t numElements, size_t structSize, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default)
            {
                return GpuResourceDescription(numElements * structSize, structSize, BufferFlags::StructuredBuffer, bindFlags, usage);
            }

            static GpuResourceDescription IndexBuffer(size_t numElements, GAPI::GpuResourceFormat format, GpuResourceUsage usage = GpuResourceUsage::Default)
            {
                ASSERT_MSG(format == GAPI::GpuResourceFormat::R16Uint || format == GAPI::GpuResourceFormat::R32Uint, "Only R16Uint and R32Uint formats are allowed for index buffers");
                size_t elementSize = GpuResourceFormatInfo::GetBlockSize(format);
                return GpuResourceDescription(numElements * elementSize, elementSize, BufferFlags::IndexBuffer, GpuResourceBindFlags::None, usage);
            }

            static GpuResourceDescription Texture1D(uint32_t width, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture1D, width, 1, 1, format, bindFlags, usage, MultisampleType::None, arraySize, mipLevels);
            }

            static GpuResourceDescription Texture2D(uint32_t width, uint32_t height, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture2D, width, height, 1, format, bindFlags, usage, MultisampleType::None, arraySize, mipLevels);
            }

            static GpuResourceDescription Texture2DMS(uint32_t width, uint32_t height, GpuResourceFormat format, MultisampleType multisampleType, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default, uint32_t arraySize = 1)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture2DMS, width, height, 1, format, bindFlags, usage, multisampleType, arraySize, 1);
            }

            static GpuResourceDescription Texture3D(uint32_t width, uint32_t height, uint32_t depth, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::Texture3D, width, height, depth, format, bindFlags, usage, MultisampleType::None, 1, mipLevels);
            }

            static GpuResourceDescription TextureCube(uint32_t width, uint32_t height, GpuResourceFormat format, GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource, GpuResourceUsage usage = GpuResourceUsage::Default, uint32_t arraySize = 1, uint32_t mipLevels = MaxPossible)
            {
                return GpuResourceDescription(GpuResourceDimension::TextureCube, width, height, 1, format, bindFlags, usage, MultisampleType::None, arraySize, mipLevels);
            }

            inline friend bool operator==(const GpuResourceDescription& lhs, const GpuResourceDescription& rhs)
            {
                static_assert(sizeof(GpuResourceDescription) == 12 * sizeof(uint32_t), "Check for tighly packed structure");
                static_assert(sizeof(TextureDescription) == 7 * sizeof(uint32_t), "Check for tighly packed structure");
                static_assert(sizeof(BufferDescription) == 2 * sizeof(size_t) + 2 * sizeof(uint32_t), "Check for tighly packed structure");

                bool resourceDescriptionCmp = lhs.dimension == rhs.dimension &&
                                              lhs.usage == rhs.usage &&
                                              lhs.bindFlags == rhs.bindFlags;

                resourceDescriptionCmp &= lhs.dimension == GpuResourceDimension::Buffer
                                              ? std::memcmp(&lhs.buffer, &rhs.buffer, 2 * sizeof(size_t) + sizeof(uint32_t)) == 0
                                              : std::memcmp(&lhs.texture, &rhs.texture, sizeof(TextureDescription)) == 0;

                return resourceDescriptionCmp;
            }
            inline friend bool operator!=(const GpuResourceDescription& lhs, const GpuResourceDescription& rhs) { return !(lhs == rhs); }

        public:
            bool IsBuffer() const { return dimension == GpuResourceDimension::Buffer; }
            bool IsTexture() const { return dimension != GpuResourceDimension::Buffer; }

            GpuResourceDimension GetDimension() const { return dimension; }

            uint32_t GetWidth(uint32_t mipLevel = 0) const
            {
                ASSERT(dimension != GpuResourceDimension::Buffer);
                ASSERT(mipLevel < texture.mipLevels);
                return (mipLevel < texture.mipLevels) ? std::max(1u, texture.width >> mipLevel) : 0u;
            }

            uint32_t GetHeight(uint32_t mipLevel = 0) const
            {
                ASSERT(dimension != GpuResourceDimension::Buffer);
                ASSERT(mipLevel < texture.mipLevels);
                return (mipLevel < texture.mipLevels) ? std::max(1u, texture.height >> mipLevel) : 0u;
            }

            uint32_t GetDepth(uint32_t mipLevel = 0) const
            {
                ASSERT(dimension != GpuResourceDimension::Buffer);
                ASSERT(mipLevel < texture.mipLevels);
                return (mipLevel < texture.mipLevels) ? std::max(1U, texture.depth >> mipLevel) : 0u;
            }

            uint32_t GetSubresourceArraySlice(uint32_t subresource) const
            {
                ASSERT(dimension != GpuResourceDimension::Buffer);
                ASSERT(texture.mipLevels > 0);
                ASSERT(subresource < GetNumSubresources());

                const auto numFaces = (dimension == GpuResourceDimension::TextureCube ? 6 : 1);
                return (subresource / numFaces) / texture.mipLevels;
            }

            uint32_t GetSubresourceFace(uint32_t subresource) const
            {
                ASSERT(dimension != GpuResourceDimension::Buffer);
                ASSERT(texture.mipLevels > 0);
                ASSERT(subresource < GetNumSubresources());

                return (dimension == GpuResourceDimension::TextureCube ? (subresource / texture.mipLevels) % 6 : 0);
            }

            uint32_t GetSubresourceMipLevel(uint32_t subresource) const
            {
                ASSERT(dimension != GpuResourceDimension::Buffer);
                ASSERT(texture.mipLevels > 0);
                ASSERT(subresource < GetNumSubresources());

                return subresource % texture.mipLevels;
            }

            uint32_t GetSubresourceIndex(uint32_t arraySlice, uint32_t mipLevel, uint32_t faceIndex) const
            {
                ASSERT(dimension != GpuResourceDimension::Buffer);
                ASSERT(mipLevel < texture.mipLevels);
                ASSERT(arraySlice < texture.arraySize);

                const uint32_t numFaces = (dimension == GpuResourceDimension::TextureCube ? 6u : 1u);
                ASSERT(faceIndex < numFaces);

                return mipLevel + (arraySlice * numFaces + faceIndex) * texture.mipLevels;
            }

            size_t GetNumElements() const
            {
                ASSERT(dimension == GpuResourceDimension::Buffer);
                return buffer.stride > 0 ? (buffer.size / buffer.stride) : 1;
            }

            uint32_t GetNumSubresources() const
            {
                constexpr uint32_t planeSlices = 1;
                const uint32_t numFaces = (dimension == GpuResourceDimension::TextureCube ? 6 : 1);
                const uint32_t arraySize = (dimension == GpuResourceDimension::Buffer ? 1 : texture.arraySize);
                const uint32_t mipLevels = (dimension == GpuResourceDimension::Buffer ? 1 : texture.mipLevels);
                return planeSlices * numFaces * arraySize * mipLevels;
            }

            GpuResourceFormat GetIndexBufferFormat() const
            {
                ASSERT(dimension == GpuResourceDimension::Buffer);
                ASSERT(IsIndex());
                return buffer.stride == 2   ? GpuResourceFormat::R16Uint
                       : buffer.stride == 4 ? GpuResourceFormat::R32Uint
                                            : GpuResourceFormat::Unknown;
            }

            uint32_t GetMaxMipLevel() const
            {
                const uint32_t maxDimension = std::max(texture.width, std::max(texture.height, texture.depth));
                return dimension == GpuResourceDimension::Buffer ? 1 : 1 + static_cast<uint32_t>(log2(static_cast<float>(maxDimension)));
            }

            bool IsStuctured() const
            {
                ASSERT(dimension == GpuResourceDimension::Buffer);
                return IsSet(buffer.flags, BufferFlags::StructuredBuffer);
            }

            bool IsRaw() const
            {
                ASSERT(dimension == GpuResourceDimension::Buffer);
                return IsSet(buffer.flags, BufferFlags::RawBuffer);
            }

            bool IsIndex() const
            {
                ASSERT(dimension == GpuResourceDimension::Buffer);
                return IsSet(buffer.flags, BufferFlags::IndexBuffer);
            }

            bool IsValid() const;

        private:
            GpuResourceDescription(GpuResourceDimension dimension,
                                   uint32_t width, uint32_t height, uint32_t depth,
                                   GpuResourceFormat format,
                                   GpuResourceBindFlags bindFlags,
                                   GpuResourceUsage usage,
                                   MultisampleType multisampleType,
                                   uint32_t arraySize,
                                   uint32_t mipLevels)
                : dimension(dimension),
                  bindFlags(bindFlags),
                  usage(usage)
            {
                texture.format = format;
                texture.width = width;
                texture.height = height;
                texture.depth = depth;
                texture.multisampleType = multisampleType;
                texture.arraySize = arraySize;

                // Limit/Calc maximum mip count
                texture.mipLevels = (std::min(GetMaxMipLevel(), mipLevels));
                ASSERT(IsValid());
            }

            GpuResourceDescription(size_t size,
                                   size_t stride,
                                   BufferFlags bufferFlags,
                                   GpuResourceBindFlags bindFlags,
                                   GpuResourceUsage usage)
                : dimension(GpuResourceDimension::Buffer),
                  bindFlags(bindFlags),
                  usage(usage)
            {
                buffer.size = size;
                buffer.stride = stride;
                buffer.flags = bufferFlags;

                ASSERT(IsValid());
            }

        private:
            struct TextureDescription
            {
                uint32_t width = 1;
                uint32_t height = 1;
                uint32_t depth = 1;
                uint32_t mipLevels = 1;
                uint32_t arraySize = 1;
                GpuResourceFormat format = GpuResourceFormat::Unknown;
                MultisampleType multisampleType = MultisampleType::None;
            };

            struct BufferDescription
            {
                size_t size = 1;
                size_t stride = 1;
                BufferFlags flags = BufferFlags::None;
            };

        public:
            union
            {
                TextureDescription texture;
                BufferDescription buffer;
            };

            GpuResourceDimension dimension = GpuResourceDimension::Texture2D;
            GpuResourceBindFlags bindFlags = GpuResourceBindFlags::ShaderResource;
            GpuResourceUsage usage = GpuResourceUsage::Default;
        };

        struct GpuResourceFootprint
        {
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

            std::vector<SubresourceFootprint> subresourceFootprints;
            size_t totalSize;
        };

        class IGpuResource
        {
        public:
            virtual ~IGpuResource() = default;

            virtual void DestroyImmediatly() = 0; // unused
            virtual std::any GetRawHandle() const = 0; // unused
            virtual std::vector<GpuResourceFootprint::SubresourceFootprint> GetSubresourceFootprints(const GpuResourceDescription& decription) const = 0;

            virtual void* Map() = 0;
            virtual void Unmap() = 0;
        };

        class GpuResource : public Resource<IGpuResource>, public eastl::enable_shared_from_this<GpuResource>
        {
        public:
            template <typename Type>
            eastl::shared_ptr<Type> GetTyped();

            inline const GpuResourceDescription& GetDescription() const { return description_; }
            inline IDataBuffer::SharedPtr GetInitialData() const { return initialData_; }
            inline std::vector<GpuResourceFootprint::SubresourceFootprint> GetSubresourceFootprints() const
            {
                return GetPrivateImpl()->GetSubresourceFootprints(description_);
            }

            // TODO Temporary
            inline std::any GetRawHandle() const { return GetPrivateImpl()->GetRawHandle(); }

            inline void ResetInitialData()
            {
                ASSERT(initialData_);
                initialData_.reset();
            }

            inline void* Map() { return GetPrivateImpl()->Map(); }
            inline void Unmap() { return GetPrivateImpl()->Unmap(); }

        protected:
            GpuResource(GpuResourceDescription description,
                        IDataBuffer::SharedPtr initialData,
                        const std::string& name)
                : Resource(Type::GpuResource, name),
                  description_(description),
                  initialData_(initialData) { };

        protected:
            GpuResourceDescription description_;
            IDataBuffer::SharedPtr initialData_;

            // TODO NOT UNORDERD MAP NEVER EVER USE IT!!!!!
            std::unordered_map<GpuResourceViewDescription, eastl::unique_ptr<ShaderResourceView>, GpuResourceViewDescription::HashFunc> srvs_;
            std::unordered_map<GpuResourceViewDescription, eastl::unique_ptr<RenderTargetView>, GpuResourceViewDescription::HashFunc> rtvs_;
            std::unordered_map<GpuResourceViewDescription, eastl::unique_ptr<DepthStencilView>, GpuResourceViewDescription::HashFunc> dsvs_;
            std::unordered_map<GpuResourceViewDescription, eastl::unique_ptr<UnorderedAccessView>, GpuResourceViewDescription::HashFunc> uavs_;
        };

        template <>
        eastl::shared_ptr<Texture> GpuResource::GetTyped<Texture>();

        template <>
        eastl::shared_ptr<Buffer> GpuResource::GetTyped<Buffer>();
/*
        class GpuResourceDataGuard : public IDataBuffer
        {
        public:
            GpuResourceDataGuard(const GpuResource::SharedPtr& resource)
                : size_(0), resource_(resource)
            {
                data_ = resource_->Map();
            }

            ~GpuResourceDataGuard() { resource_->Unmap(); }

            size_t Size() const override { return size_; }
            void* Data() const override { return data_; }

        private:
            size_t size_;
            void* data_;
            GpuResource::SharedPtr resource_;
        };*/
    }
}