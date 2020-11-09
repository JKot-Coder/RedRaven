#pragma once

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class Resource;
        using ResourceWeakPtr = std::weak_ptr<Resource>;

        class ResourceView : public Object
        {
        public:
            enum class Type
            {
                RenderTargetView
            };

        public:
            using SharedPtr = std::shared_ptr<ResourceView>;
            using SharedConstPtr = std::shared_ptr<const ResourceView>;

        protected:
            ResourceView(Type type, const ResourceWeakPtr& resource, const U8String& name)
                : Object(Object::Type::ResourceView, name),
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

            static SharedPtr Create(const ResourceWeakPtr& resource, const U8String& name)
            {
                return SharedPtr(new RenderTargetView(resource, name));
            }

        private:
            RenderTargetView(const ResourceWeakPtr& resource, const U8String& name)
                : ResourceView(ResourceView::Type::RenderTargetView, resource, name) { }
        };

    }
}