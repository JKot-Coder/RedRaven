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
            using UniquePtr = eastl::unique_ptr<GpuResourceView>;

            enum class ViewType
            {
                ShaderResourceView,
                DepthStencilView,
                RenderTargetView,
                UnorderedAccessView,
            };

            ViewType GetViewType() const { return viewType_; }
            const GpuResourceViewDescription& GetDescription() const { return description_; }
            const GpuResource* GetGpuResource() const { return gpuResource_; }

        protected:
            GpuResourceView(ViewType viewType, const GpuResource& gpuResource, const GpuResourceViewDescription& description)
                : Resource<IGpuResourceView, false>(Type::GpuResourceView),
                  viewType_(viewType),
                  description_(description),
                  gpuResource_(&gpuResource)
            {
            }

        private:
            ViewType viewType_;
            GpuResourceViewDescription description_;
            const GpuResource* gpuResource_;
        };

        class ShaderResourceView final : public GpuResourceView
        {
        public:
            using UniquePtr = eastl::unique_ptr<ShaderResourceView>;

        private:
            static UniquePtr Create(
                const GpuResource& gpuResource,
                const GpuResourceViewDescription& desc)
            {
                return UniquePtr(new ShaderResourceView(gpuResource, desc));
            };

            ShaderResourceView(const GpuResource& gpuResource, const GpuResourceViewDescription& desc);

            friend class Render::DeviceContext;
            friend class Render::DeviceContext;
        };

        class DepthStencilView final : public GpuResourceView
        {
        public:
            using UniquePtr = eastl::unique_ptr<DepthStencilView>;

        private:
            static UniquePtr Create(
                const Texture& texture,
                const GpuResourceViewDescription& desc)
            {
                return UniquePtr(new DepthStencilView(texture, desc));
            };

            DepthStencilView(const Texture& texture, const GpuResourceViewDescription& desc);

            friend class Render::DeviceContext;
            friend class Render::DeviceContext;
        };

        class RenderTargetView final : public GpuResourceView
        {
        public:
            using UniquePtr = eastl::unique_ptr<RenderTargetView>;

        private:
            static UniquePtr Create(
                const Texture& texture,
                const GpuResourceViewDescription& desc)
            {
                return UniquePtr(new RenderTargetView(texture, desc));
            };

            RenderTargetView(const Texture& texture, const GpuResourceViewDescription& desc);

            friend class Render::DeviceContext;
            friend class Render::DeviceContext;
        };

        class UnorderedAccessView final : public GpuResourceView
        {
        public:
            using UniquePtr = eastl::unique_ptr<UnorderedAccessView>;

        private:
            static UniquePtr Create(
                const GpuResource& gpuResource,
                const GpuResourceViewDescription& desc)
            {
                return UniquePtr(new UnorderedAccessView(gpuResource, desc));
            };

            UnorderedAccessView(const GpuResource& gpuResource, const GpuResourceViewDescription& desc);

            friend class Render::DeviceContext;
            friend class Render::DeviceContext;
        };
    }
}