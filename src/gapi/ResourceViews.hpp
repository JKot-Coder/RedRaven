#pragma once

#include "gapi/Object.hpp"

#include "gapi/ForwardDeclarations.hpp"

namespace OpenDemo
{
    namespace GAPI
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

        class ResourceView : public Object
        {
        public:
            using SharedPtr = std::shared_ptr<ResourceView>;
            using SharedConstPtr = std::shared_ptr<const ResourceView>;

            enum class ViewType
            {
                RenderTargetView,
                ShaderResourceView,
                DepthStencilView,
                UnorderedAccessView,
            };

            ViewType GetViewType() const { return viewType_; }
            const ResourceViewDescription& GetDescription() const { return description_; }
            std::weak_ptr<Resource> GetResource() const { return resource_; }

        protected:
            ResourceView(ViewType viewType, const std::weak_ptr<Resource>& resource, const ResourceViewDescription& description, const U8String& name)
                : Object(Object::Type::ResourceView, name),
                  viewType_(viewType),
                  resource_(resource),
                  description_(description)
            {
                ASSERT(!resource_.expired())
            }

        private:
            ViewType viewType_;
            ResourceViewDescription description_;
            std::weak_ptr<Resource> resource_;
        };

        class RenderTargetView final : public ResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<RenderTargetView>;
            using SharedConstPtr = std::shared_ptr<RenderTargetView>;

        private:
            static SharedPtr Create(const std::shared_ptr<Texture>& texture, const ResourceViewDescription& desc, const U8String& name);

            RenderTargetView(const std::weak_ptr<Resource>& resource, const ResourceViewDescription& desc, const U8String& name)
                : ResourceView(ResourceView::ViewType::RenderTargetView, resource, desc, name) { }

            friend class Render::RenderContext;
        };
    }
}