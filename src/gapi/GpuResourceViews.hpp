#pragma once

#include "gapi/Resource.hpp"

#include "gapi/ForwardDeclarations.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        struct GpuResourceViewDescription
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
            GpuResourceViewDescription(uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount)
                : texture({ mipLevel, mipsCount, firstArraySlice, arraySlicesCount })
            {
            }

            GpuResourceViewDescription(uint32_t firstElement, uint32_t elementsCount)
                : buffer({ firstElement, elementsCount })
            {
            }
        };

        class GpuResourceViewInterface
        {
        public:
            virtual ~GpuResourceViewInterface() {};
        };

        class GpuResourceView : public Resource<GpuResourceViewInterface>
        {
        public:
            using SharedPtr = std::shared_ptr<GpuResourceView>;
            using SharedConstPtr = std::shared_ptr<const GpuResourceView>;

            enum class ViewType
            {
                RenderTargetView,
                ShaderGpuResourceView,
                DepthStencilView,
                UnorderedAccessView,
            };

            ViewType GetViewType() const { return viewType_; }
            const GpuResourceViewDescription& GetDescription() const { return description_; }
            std::weak_ptr<GpuResource> GetGpuResource() const { return gpuResource_; }

        protected:
            GpuResourceView(ViewType viewType, const std::weak_ptr<GpuResource>& gpuResource, const GpuResourceViewDescription& description, const U8String& name)
                : Resource(Object::Type::GpuResourceView, name),
                  viewType_(viewType),
                  gpuResource_(gpuResource),
                  description_(description)
            {
                ASSERT(!gpuResource_.expired())
            }

        private:
            ViewType viewType_;
            GpuResourceViewDescription description_;
            std::weak_ptr<GpuResource> gpuResource_;
        };

        class RenderTargetView final : public GpuResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<RenderTargetView>;
            using SharedConstPtr = std::shared_ptr<RenderTargetView>;

        private:
            static SharedPtr Create(const std::shared_ptr<Texture>& texture, const GpuResourceViewDescription& desc, const U8String& name);

            RenderTargetView(const std::weak_ptr<GpuResource>& GpuResource, const GpuResourceViewDescription& desc, const U8String& name)
                : GpuResourceView(GpuResourceView::ViewType::RenderTargetView, GpuResource, desc, name) { }

            friend class Render::RenderContext;
        };
    }
}