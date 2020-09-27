#pragma once

#include "gapi/Resource.hpp"
#include "gapi/Texture.hpp"

namespace OpenDemo
{
    namespace Render
    {

        class ResourceView : public Resource
        {
        public:
            enum class Type
            {
                RenderTargetView
            };

        public:
            using SharedPtr = std::shared_ptr<ResourceView>;
            using SharedConstPtr = std::shared_ptr<const ResourceView>;

            ResourceView(const ResourceView&) = delete;
            ResourceView& operator=(const ResourceView&) = delete;

        protected:
            ResourceView(Type type, const ResourceWeakPtr& resource, const U8String& name)
                : Resource(Resource::Type::ResourceView, name),
                  resource_(resource),
                  type_(type)
            {
                ASSERT(!resource_.expired())
            }

        private:
            Type type_;
            ResourceWeakPtr resource_;
        };

        class RenderTargetView final : public ResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<RenderTargetView>;
            using SharedConstPtr = std::shared_ptr<const RenderTargetView>;

            RenderTargetView(const RenderTargetView&) = delete;
            RenderTargetView& operator=(const RenderTargetView&) = delete;

            static SharedPtr create(Texture::ConstSharedPtrRef texture, const U8String& name)
            {
                ASSERT(texture)
                return SharedPtr(new RenderTargetView(texture, name));
            }

        private:
            RenderTargetView(const ResourceWeakPtr& resource, const U8String& name)
                : ResourceView(ResourceView::Type::RenderTargetView, resource, name) { }
        };

    }
}