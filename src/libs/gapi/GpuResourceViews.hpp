#pragma once

#include "gapi/ForwardDeclarations.hpp"

#include "gapi/Resource.hpp"

namespace RR
{
    namespace GAPI
    {
        struct GpuResourceViewDescription
        {
        public:
            static GpuResourceViewDescription Buffer(GpuResourceFormat format, size_t firstElement, size_t elementsCount)
            {
                return GpuResourceViewDescription(format, firstElement, elementsCount);
            }

            static GpuResourceViewDescription Texture(GpuResourceFormat format, uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount)
            {
                return GpuResourceViewDescription(format, mipLevel, mipsCount, firstArraySlice, arraySlicesCount);
            }

            bool operator==(const GpuResourceViewDescription& other) const
            {
                static_assert(sizeof(GpuResourceViewDescription) == 24);
                return (format == other.format) &&
                       (texture.mipLevel == other.texture.mipLevel) &&
                       (texture.mipCount == other.texture.mipCount) &&
                       (texture.firstArraySlice == other.texture.firstArraySlice) &&
                       (texture.arraySliceCount == other.texture.arraySliceCount);
            }

            struct HashFunc
            {
                std::size_t operator()(const GpuResourceViewDescription& desc) const
                {
                    static_assert(sizeof(GpuResourceViewDescription) == 24);
                    return (std::hash<uint32_t>()(static_cast<uint32_t>(desc.format))) ^ // TODO StdHash?
                           (std::hash<uint32_t>()(desc.texture.firstArraySlice) << 1) ^
                           (std::hash<uint32_t>()(desc.texture.arraySliceCount) << 3) ^
                           (std::hash<uint32_t>()(desc.texture.mipCount) << 5) ^
                           (std::hash<uint32_t>()(desc.texture.mipLevel) << 7); // TODO This is ok?
                }
            };

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
                    size_t firstElement;
                    size_t elementCount;
                } buffer;
            };

            GpuResourceFormat format;

        private:
            GpuResourceViewDescription(GpuResourceFormat format, uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount);
            GpuResourceViewDescription(GpuResourceFormat format, size_t firstElement, size_t elementsCount);
        };

        class IGpuResourceView
        {
        public:
            virtual ~IGpuResourceView() {};
        };

        class GpuResourceView : public Resource<IGpuResourceView, false>
        {
        public:
            using SharedPtr = eastl::shared_ptr<GpuResourceView>;
            using SharedConstPtr = eastl::shared_ptr<const GpuResourceView>;

            enum class ViewType
            {
                ShaderResourceView,
                DepthStencilView,
                RenderTargetView,
                UnorderedAccessView,
            };

            ViewType GetViewType() const { return viewType_; }
            const GpuResourceViewDescription& GetDescription() const { return description_; }
            eastl::weak_ptr<GpuResource> GetGpuResource() const { return gpuResource_; }

        protected:
            GpuResourceView(ViewType viewType, const eastl::weak_ptr<GpuResource>& gpuResource, const GpuResourceViewDescription& description)
                : Resource<IGpuResourceView, false>(Object::Type::GpuResourceView),
                  viewType_(viewType),
                  description_(description),
                  gpuResource_(gpuResource)
            {
                ASSERT(!gpuResource_.expired());
            }

        private:
            ViewType viewType_;
            GpuResourceViewDescription description_;
            eastl::weak_ptr<GpuResource> gpuResource_;
        };

        class ShaderResourceView final : public GpuResourceView
        {
        public:
            using SharedPtr = eastl::shared_ptr<ShaderResourceView>;
            using SharedConstPtr = eastl::shared_ptr<ShaderResourceView>;

        private:
            static SharedPtr Create(
                const eastl::weak_ptr<GpuResource>& gpuResource,
                const GpuResourceViewDescription& desc)
            {
                return SharedPtr(new ShaderResourceView(gpuResource, desc));
            };

            ShaderResourceView(const eastl::weak_ptr<GpuResource>& gpuResource, const GpuResourceViewDescription& desc);

            friend class Render::DeviceContext;
            friend class RenderLoom::DeviceContext;
        };

        class DepthStencilView final : public GpuResourceView
        {
        public:
            using SharedPtr = eastl::shared_ptr<DepthStencilView>;
            using SharedConstPtr = eastl::shared_ptr<DepthStencilView>;

        private:
            static SharedPtr Create(
                const eastl::weak_ptr<Texture>& texture,
                const GpuResourceViewDescription& desc)
            {
                return SharedPtr(new DepthStencilView(texture, desc));
            };

            DepthStencilView(const eastl::weak_ptr<Texture>& texture, const GpuResourceViewDescription& desc);

            friend class Render::DeviceContext;
            friend class RenderLoom::DeviceContext;
        };

        class RenderTargetView final : public GpuResourceView
        {
        public:
            using SharedPtr = eastl::shared_ptr<RenderTargetView>;
            using SharedConstPtr = eastl::shared_ptr<RenderTargetView>;

        private:
            static SharedPtr Create(
                const eastl::shared_ptr<Texture>& texture,
                const GpuResourceViewDescription& desc)
            {
                return SharedPtr(new RenderTargetView(texture, desc));
            };

            RenderTargetView(const eastl::weak_ptr<Texture>& texture, const GpuResourceViewDescription& desc);

            friend class Render::DeviceContext;
            friend class RenderLoom::DeviceContext;
        };

        class UnorderedAccessView final : public GpuResourceView
        {
        public:
            using SharedPtr = eastl::shared_ptr<UnorderedAccessView>;
            using SharedConstPtr = eastl::shared_ptr<UnorderedAccessView>;

        private:
            static SharedPtr Create(
                const eastl::shared_ptr<GpuResource>& gpuResource,
                const GpuResourceViewDescription& desc)
            {
                return SharedPtr(new UnorderedAccessView(gpuResource, desc));
            };

            UnorderedAccessView(const eastl::weak_ptr<GpuResource>& gpuResource, const GpuResourceViewDescription& desc);

            friend class Render::DeviceContext;
            friend class RenderLoom::DeviceContext;
        };
    }
}