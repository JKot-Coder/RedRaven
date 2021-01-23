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
            Alpha32Float,

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

            enum class Type
            {
                Buffer,
                Texture
            };

        public:
            inline GpuResource::Type GetGpuResourceType() const { return type_; }

            template <typename Type>
            std::shared_ptr<Type> GetTyped();

            inline GpuResourceBindFlags GetBindFlags() const { return bindFlags_; }

        protected:
            GpuResource(GpuResource::Type type, GpuResourceBindFlags bindFlags, const U8String& name)
                : Resource(Object::Type::GpuResource, name),
                  type_(type),
                  bindFlags_(bindFlags)
            {
            }

            GpuResource::Type type_;
            GpuResourceBindFlags bindFlags_;

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