#pragma once

#include "gapi/ForwardDeclarations.hpp"

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        struct GpuResourceViewDescription
        {
        public:
            static GpuResourceViewDescription Buffer(GpuResourceFormat format, uint32_t firstElement, uint32_t elementsCount)
            {
                return GpuResourceViewDescription(format, firstElement, elementsCount);
            }

            static GpuResourceViewDescription Texture(GpuResourceFormat format, uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount)
            {
                return GpuResourceViewDescription(format, mipLevel, mipsCount, firstArraySlice, arraySlicesCount);
            }

            bool operator==(const GpuResourceViewDescription& other) const
            {
                static_assert(sizeof(GpuResourceViewDescription) == 20);
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
                    static_assert(sizeof(GpuResourceViewDescription) == 20);
                    return (std::hash<uint32_t>()(static_cast<uint32_t>(desc.format))) ^
                           (std::hash<uint32_t>()(desc.texture.firstArraySlice) << 1) ^
                           (std::hash<uint32_t>()(desc.texture.arraySliceCount) << 3) ^
                           (std::hash<uint32_t>()(desc.texture.mipCount) << 5) ^
                           (std::hash<uint32_t>()(desc.texture.mipLevel) << 7);
                }
            };

        public:
            union
            {
                struct
                {
                    uint32_t mipLevel;
                    uint32_t mipCount;
                    uint32_t firstArraySlice = 0;
                    uint32_t arraySliceCount = 0;
                } texture;

                struct
                {
                    uint32_t firstElement;
                    uint32_t elementCount;
                } buffer;
            };

            GpuResourceFormat format;

        private:
            GpuResourceViewDescription(GpuResourceFormat format, uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount);
            GpuResourceViewDescription(GpuResourceFormat format, uint32_t firstElement, uint32_t elementsCount);
        };

        class IGpuResourceView
        {
        public:
            virtual ~IGpuResourceView() {};
        };

        class GpuResourceView : public Resource<IGpuResourceView, false>
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
            GpuResourceView(ViewType viewType, const std::weak_ptr<GpuResource>& gpuResource, const GpuResourceViewDescription& description)
                : Resource<IGpuResourceView, false>(Object::Type::GpuResourceView),
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
            static SharedPtr Create(
                const std::weak_ptr<GpuResource>& gpuResource,
                const GpuResourceViewDescription& desc)
            {
                return SharedPtr(new ShaderResourceView(gpuResource, desc));
            };

            ShaderResourceView(const std::weak_ptr<GpuResource>& gpuResource, const GpuResourceViewDescription& desc);
            friend class Render::DeviceContext;
        };

        class DepthStencilView final : public GpuResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<DepthStencilView>;
            using SharedConstPtr = std::shared_ptr<DepthStencilView>;

        private:
            static SharedPtr Create(
                const std::weak_ptr<Texture>& texture,
                const GpuResourceViewDescription& desc)
            {
                return SharedPtr(new DepthStencilView(texture, desc));
            };

            DepthStencilView(const std::weak_ptr<Texture>& texture, const GpuResourceViewDescription& desc);

            friend class Render::DeviceContext;
        };

        class RenderTargetView final : public GpuResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<RenderTargetView>;
            using SharedConstPtr = std::shared_ptr<RenderTargetView>;

        private:
            static SharedPtr Create(
                const std::shared_ptr<Texture>& texture,
                const GpuResourceViewDescription& desc)
            {
                return SharedPtr(new RenderTargetView(texture, desc));
            };

            RenderTargetView(const std::weak_ptr<Texture>& texture, const GpuResourceViewDescription& desc);

            friend class Render::DeviceContext;
        };

        class UnorderedAccessView final : public GpuResourceView
        {
        public:
            using SharedPtr = std::shared_ptr<UnorderedAccessView>;
            using SharedConstPtr = std::shared_ptr<UnorderedAccessView>;

        private:
            static SharedPtr Create(
                const std::shared_ptr<GpuResource>& gpuResource,
                const GpuResourceViewDescription& desc)
            {
                return SharedPtr(new UnorderedAccessView(gpuResource, desc));
            };

            UnorderedAccessView(const std::weak_ptr<GpuResource>& gpuResource, const GpuResourceViewDescription& desc);

            friend class Render::DeviceContext;
        };
    }
}