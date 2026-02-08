#pragma once

#include "gapi/ForwardDeclarations.hpp"

#include "gapi/Resource.hpp"

namespace RR
{
    namespace GAPI
    {
        struct GpuResourceViewDesc
        {
        public:
            static GpuResourceViewDesc Buffer(GpuResourceFormat format, size_t firstElement, size_t elementsCount)
            {
                return GpuResourceViewDesc(format, firstElement, elementsCount);
            }

            static GpuResourceViewDesc Texture(GpuResourceFormat format, uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount)
            {
                return GpuResourceViewDesc(format, mipLevel, mipsCount, firstArraySlice, arraySlicesCount);
            }

            bool operator==(const GpuResourceViewDesc& other) const
            {
                static_assert(sizeof(GpuResourceViewDesc) == 24);
                return (format == other.format) &&
                       (texture.mipLevel == other.texture.mipLevel) &&
                       (texture.mipCount == other.texture.mipCount) &&
                       (texture.firstArraySlice == other.texture.firstArraySlice) &&
                       (texture.arraySliceCount == other.texture.arraySliceCount);
            }

            struct HashFunc
            {
                std::size_t operator()(const GpuResourceViewDesc& desc) const
                {
                    static_assert(sizeof(GpuResourceViewDesc) == 24);
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
            GpuResourceViewDesc(GpuResourceFormat format, uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount);
            GpuResourceViewDesc(GpuResourceFormat format, size_t firstElement, size_t elementsCount);
        };

        class IGpuResourceView
        {
        public:
            virtual ~IGpuResourceView() {};
        };

        class GpuResourceView : public Resource<IGpuResourceView, false>
        {
        public:
            enum class ViewType
            {
                ShaderResourceView,
                DepthStencilView,
                RenderTargetView,
                UnorderedAccessView,
            };

            ViewType GetViewType() const { return viewType_; }
            const GpuResourceViewDesc& GetDesc() const { return desc_; }
            GAPI::GpuResource* GetGpuResource() const { return gpuResource_; }

        protected:
            GpuResourceView(ViewType viewType, GAPI::GpuResource& gpuResource, const GpuResourceViewDesc& desc)
                : Resource<IGpuResourceView, false>(Type::GpuResourceView),
                  viewType_(viewType),
                  desc_(desc),
                  gpuResource_(&gpuResource)
            {
            }

        private:
            ViewType viewType_;
            GpuResourceViewDesc desc_;
            GAPI::GpuResource* gpuResource_;
        };

        class ShaderResourceView final : public GpuResourceView
        {
        public:
            using UniquePtr = eastl::unique_ptr<ShaderResourceView>;

        private:
            ShaderResourceView(GAPI::GpuResource& gpuResource, const GpuResourceViewDesc& desc);

            friend class Render::DeviceContext;
        };

        class DepthStencilView final : public GpuResourceView
        {
        public:
            using UniquePtr = eastl::unique_ptr<DepthStencilView>;

        private:
            DepthStencilView(GAPI::Texture& texture, const GpuResourceViewDesc& desc);

            friend class Render::DeviceContext;
        };

        class RenderTargetView final : public GpuResourceView
        {
        public:
            using UniquePtr = eastl::unique_ptr<RenderTargetView>;

        private:
            RenderTargetView(GAPI::Texture& texture, const GpuResourceViewDesc& desc);

            friend class Render::DeviceContext;
        };

        class UnorderedAccessView final : public GpuResourceView
        {
        public:
            using UniquePtr = eastl::unique_ptr<UnorderedAccessView>;

        private:
            UnorderedAccessView(GAPI::GpuResource& gpuResource, const GpuResourceViewDesc& desc);

            friend class Render::DeviceContext;
        };
    }
}