#pragma once

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class Resource;
        class Texture;
        class Buffer;
        using ResourceWeakPtr = std::weak_ptr<Resource>;
        using ConstTextureSharedPtrRef = const std::shared_ptr<Texture>&;
        using ConstBufferSharedPtrRef = const std::shared_ptr<Buffer>&;

        namespace
        {
            struct ResourceViewDescription
            {
                union
                {
                    struct
                    {
                        uint32_t mipLevel;
                        uint32_t firstArraySlice;
                        uint32_t numArraySlices;
                    } texture;

                    struct
                    {
                        uint32_t firstElement;
                        uint32_t numElements;
                    } buffer;
                };

            public:
                ResourceViewDescription(uint32_t mipLevel, uint32_t firstArraySlice, uint32_t numArraySlices)
                    : texture({ mipLevel, firstArraySlice, numArraySlices })
                {
                }

                ResourceViewDescription(uint32_t firstElement, uint32_t numElements)
                    : buffer({ firstElement, numElements })
                {
                }
            };
        };

        class ResourceView : public Object
        {
        public:
            using SharedPtr = std::shared_ptr<ResourceView>;
            using SharedConstPtr = std::shared_ptr<const ResourceView>;
            using Description = ResourceViewDescription;

            enum class ViewType
            {
                RenderTargetView,
                ShaderResourceView,
                UnorderedResourceView,
            };

        protected:
            ResourceView(ViewType viewType, const ResourceWeakPtr& resource, const Description& description, const U8String& name)
                : Object(Object::Type::ResourceView, name),
                  viewType_(viewType),
                  resource_(resource),
                  description_(description)
            {
                ASSERT(!resource_.expired())
            }

        private:
            ViewType viewType_;
            Description description_;
            ResourceWeakPtr resource_;
        };

        template <typename type>
        class ResourceViewTemplate final : public ResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<ResourceViewTemplate<type>>;
            using SharedConstPtr = std::shared_ptr<const ResourceViewTemplate<type>>;

            static SharedPtr Create(ConstTextureSharedPtrRef texture, const ResourceView::Description& desc, const U8String& name)
            {
                return SharedPtr(new ResourceViewTemplate<type>(texture, desc, name));
            }

        private:
            ResourceViewTemplate(const ResourceWeakPtr& resource, const ResourceView::Description& desc, const U8String& name)
                : ResourceView(ResourceView::ViewType::RenderTargetView, resource, desc, name) { }
        };

        using RenderTargetView = ResourceViewTemplate<void>;
        using DepthStencilView = ResourceViewTemplate<void>;
        using ShaderResourceView = ResourceViewTemplate<void>;
        using UnorderedAccessView = ResourceViewTemplate<void>;

        /*
        class RenderTargetView final : public ResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<RenderTargetView>;
            using SharedConstPtr = std::shared_ptr<const RenderTargetView>;

            static SharedPtr Create(const Texture::ConstSharedPtrRef texture, const ResourceView::Description& desc, const U8String& name)
            {
                return SharedPtr(new RenderTargetView(texture, desc, name));
            }

        private:
            RenderTargetView(const ResourceWeakPtr& resource, const ResourceView::Description& desc, const U8String& name)
                : ResourceView(ResourceView::ViewType::RenderTargetView, resource, desc, name) { }
        };

        class ShaderResourceView final : public ResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<ShaderResourceView>;
            using SharedConstPtr = std::shared_ptr<const ShaderResourceView>;

            static SharedPtr Create(const Texture::ConstSharedPtrRef texture, const ResourceView::Description& desc, const U8String& name)
            {
                return SharedPtr(new ShaderResourceView(texture, desc, name));
            }

        private:
            ShaderResourceView(const ResourceWeakPtr& resource, const ResourceView::Description& desc, const U8String& name)
                : ResourceView(ResourceView::ViewType::RenderTargetView, resource, desc, name) { }
        };

        class DepthStencilView final : public ResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<RenderTargetView>;
            using SharedConstPtr = std::shared_ptr<const RenderTargetView>;

            static SharedPtr Create(const Texture::ConstSharedPtrRef texture, const ResourceView::Description& desc, const U8String& name)
            {
                return SharedPtr(new RenderTargetView(texture, desc, name));
            }

        private:
            RenderTargetView(const ResourceWeakPtr& resource, const ResourceView::Description& desc, const U8String& name)
                : ResourceView(ResourceView::ViewType::RenderTargetView, resource, desc, name) { }
        };*/

    }
}