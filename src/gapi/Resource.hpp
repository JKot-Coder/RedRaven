#pragma once

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

            enum class BindFlags
            {
                ShaderResource
            };

            enum class Type
            {
                Buffer,
                Texture
            };

            enum class Format : uint32_t
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
                D32FloatS8X24,
                D32FloatS8X24Uint,
                D32Float,
                D24UnormS8Uint,
                D16Unorm,

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

                Count
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

        template <>
        Texture& Resource::GetTyped<Texture>();
        /*
        template <>
        Buffer& Resource::GetTyped<Buffer>();*/
    }
}