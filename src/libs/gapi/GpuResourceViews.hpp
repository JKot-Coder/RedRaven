#pragma once

#include "gapi/Resource.hpp"

#include "gapi/ForwardDeclarations.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        struct GpuResourceViewDescription
        {
            static constexpr uint32_t MaxPossible = 0xFFFFFF;

            GpuResourceViewDescription(uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount)
                : texture({ mipLevel, mipsCount, firstArraySlice, arraySlicesCount })
            {
            }

            GpuResourceViewDescription(uint32_t firstElement, uint32_t elementsCount)
                : buffer({ firstElement, elementsCount })
            {
            }

            bool operator==(const GpuResourceViewDescription& other) const
            {
                return (texture.mipLevel == other.texture.mipLevel) &&
                       (texture.mipCount == other.texture.mipCount) &&
                       (texture.firstArraySlice == other.texture.firstArraySlice) &&
                       (texture.arraySliceCount == other.texture.arraySliceCount);
            }

        public:
            union
            {
                struct
                {
                    uint32_t mipLevel;
                    uint32_t mipCount;
                    uint32_t firstArraySlice;
                    uint32_t arraySliceCount;
                } texture;

                struct
                {
                    uint32_t firstElement;
                    uint32_t elementCount;
                } buffer;
            };
        };

        class IGpuResourceView
        {
        public:
            virtual ~IGpuResourceView() {};
        };

        class GpuResourceView : public Resource<IGpuResourceView>
        {
        public:
            using SharedPtr = std::shared_ptr<GpuResourceView>;
            using SharedConstPtr = std::shared_ptr<const GpuResourceView>;

            enum class ViewType
            {
                ShaderResourceView,
                DepthStencilView,
                RenderTargetView,
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

        class ShaderResourceView final : public GpuResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<ShaderResourceView>;
            using SharedConstPtr = std::shared_ptr<ShaderResourceView>;

        private:
            template <class Deleter>
            static SharedPtr Create(
                const std::weak_ptr<GpuResource>& gpuResource,
                const GpuResourceViewDescription& desc,
                const U8String& name,
                Deleter)
            {
                return SharedPtr(new ShaderResourceView(gpuResource, desc, name), Deleter());
            };

            ShaderResourceView(const std::weak_ptr<GpuResource>& gpuResource, const GpuResourceViewDescription& desc, const U8String& name);
            friend class Render::RenderContext;
        };

        class DepthStencilView final : public GpuResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<DepthStencilView>;
            using SharedConstPtr = std::shared_ptr<DepthStencilView>;

        private:
            template <class Deleter>
            static SharedPtr Create(
                const std::weak_ptr<Texture>& texture,
                const GpuResourceViewDescription& desc,
                const U8String& name,
                Deleter)
            {
                return SharedPtr(new DepthStencilView(texture, desc, name), Deleter());
            };

            DepthStencilView(const std::weak_ptr<Texture>& texture, const GpuResourceViewDescription& desc, const U8String& name);

            friend class Render::RenderContext;
        };

        class RenderTargetView final : public GpuResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<RenderTargetView>;
            using SharedConstPtr = std::shared_ptr<RenderTargetView>;

        private:
            template <class Deleter>
            static SharedPtr Create(
                const std::shared_ptr<Texture>& texture,
                const GpuResourceViewDescription& desc,
                const U8String& name,
                Deleter)
            {
                return SharedPtr(new RenderTargetView(texture, desc, name), Deleter());
            };

            RenderTargetView(const std::weak_ptr<Texture>& texture, const GpuResourceViewDescription& desc, const U8String& name);

            friend class Render::RenderContext;
        };

        class UnorderedAccessView final : public GpuResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<UnorderedAccessView>;
            using SharedConstPtr = std::shared_ptr<UnorderedAccessView>;

        private:
            template <class Deleter>
            static SharedPtr Create(
                const std::shared_ptr<GpuResource>& gpuResource,
                const GpuResourceViewDescription& desc,
                const U8String& name,
                Deleter)
            {
                return SharedPtr(new UnorderedAccessView(gpuResource, desc, name), Deleter());
            };

            UnorderedAccessView(const std::weak_ptr<GpuResource>& gpuResource, const GpuResourceViewDescription& desc, const U8String& name);

            friend class Render::RenderContext;
        };
    }
}