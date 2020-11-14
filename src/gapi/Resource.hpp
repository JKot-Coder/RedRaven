#pragma once

#include "common/EnumClassOperators.hpp"

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class Texture;
        class Buffer;

        class Resource : public Object
        {
        public:
            using SharedPtr = std::shared_ptr<Resource>;
            using SharedConstPtr = std::shared_ptr<const Resource>;
            using ConstSharedPtrRef = const SharedPtr&;
            using WeakPtr = std::weak_ptr<Resource>;

            struct Format final
            {
            public:
                enum Value : uint32_t
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
                Format() = default;

                constexpr Format(Value value) : value_(value)
                {
                }

                operator Value() const
                {
                    return value_;
                } // Allow switch and comparisons.

            public:
                inline bool IsValid() const { return value_ > 0 && value_ < Count; };

                bool IsDepth() const;
                bool IsStencil() const;
                bool IsCompressed() const;

                uint32_t GetBlockSize() const;
                uint32_t GetCompressionBlockWidth() const;
                uint32_t GetCompressionBlockHeight() const;

                U8String ToString() const;

            private:
                Value value_ = Unknown;
            };

            enum class BindFlags : uint32_t
            {
                None = 0x0,
                ShaderResource = 0x01,
                UnorderedAccess = 0x02,
                RenderTarget = 0x04,
                DepthStencil = 0x08,
            };

            enum class Type
            {
                Buffer,
                Texture
            };

        public:
            Resource::Type inline GetResourceType() const { return resourceType_; }

            template <typename Type>
            Type& GetTyped();

        protected:
            Resource(Resource::Type resourceType, const U8String& name)
                : Object(Object::Type::Resource, name),
                  resourceType_(resourceType)
            {
            }

            Resource::Type resourceType_;
        };

        ENUM_CLASS_OPERATORS(Resource::BindFlags)

        template <>
        Texture& Resource::GetTyped<Texture>();
        /*
        template <>
        Buffer& Resource::GetTyped<Buffer>();*/

    }

}