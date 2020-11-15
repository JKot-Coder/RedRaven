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

        namespace Private
        {
            struct ResourceViewDescription
            {
                union
                {
                    struct
                    {
                        uint32_t mipLevel;
                        uint32_t mipsCount;
                        uint32_t firstArraySlice;
                        uint32_t arraySlicesCount;
                    } texture;

                    struct
                    {
                        uint32_t firstElement;
                        uint32_t elementsCount;
                    } buffer;
                };

            public:
                ResourceViewDescription(uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount)
                    : texture({ mipLevel, mipsCount, firstArraySlice, arraySlicesCount })
                {
                }

                ResourceViewDescription(uint32_t firstElement, uint32_t elementsCount)
                    : buffer({ firstElement, elementsCount })
                {
                }
            };
        };

        class ResourceView : public Object
        {
        public:
            using SharedPtr = std::shared_ptr<ResourceView>;
            using SharedConstPtr = std::shared_ptr<const ResourceView>;
            using Description = Private::ResourceViewDescription;

            enum class ViewType
            {
                RenderTargetView,
                ShaderResourceView,
                DepthStencilView,
                UnorderedAccessView,
            };

            ViewType GetViewType() const { return viewType_; }
            const Description& GetDescription() const { return description_; }
            const ResourceWeakPtr& GetResource() const { return resource_; } 

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

        template <ResourceView::ViewType type>
        class ResourceViewTemplate final : public ResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<ResourceViewTemplate<type>>;
            using SharedConstPtr = std::shared_ptr<const ResourceViewTemplate<type>>;

            friend class RenderContext;
        private:
            static SharedPtr Create(ConstTextureSharedPtrRef texture, const ResourceView::Description& desc, const U8String& name);

            ResourceViewTemplate(const ResourceWeakPtr& resource, const ResourceView::Description& desc, const U8String& name)
                : ResourceView(ResourceView::ViewType::RenderTargetView, resource, desc, name) { }
        };

        using RenderTargetView = ResourceViewTemplate<ResourceView::ViewType::RenderTargetView>;
        using DepthStencilView = ResourceViewTemplate<ResourceView::ViewType::DepthStencilView>;
        using ShaderResourceView = ResourceViewTemplate<ResourceView::ViewType::ShaderResourceView>;
        using UnorderedAccessView = ResourceViewTemplate<ResourceView::ViewType::UnorderedAccessView>;
        /*
        template <>
        RenderTargetView::SharedPtr RenderTargetView::Create(ConstTextureSharedPtrRef texture, const ResourceView::Description& desc, const U8String& name);

        template <>
        static DepthStencilView::SharedPtr DepthStencilView::Create(ConstTextureSharedPtrRef texture, const ResourceView::Description& desc, const U8String& name);

        template <>
        static ShaderResourceView::SharedPtr ShaderResourceView::Create(ConstTextureSharedPtrRef texture, const ResourceView::Description& desc, const U8String& name);*/
    }
}