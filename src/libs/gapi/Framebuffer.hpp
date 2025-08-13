#pragma once

#include "gapi/GpuResource.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/Limits.hpp"
#include "gapi/Resource.hpp"

// TODO temporary
#include <any>

namespace RR::GAPI
{
    struct FramebufferDesc
    {
    private:
        class Builder;

    public:
        static constexpr uint32_t MaxRenderTargets = MAX_RENDER_TARGETS_COUNT;

        static Builder Make();

        bool IsAnyColorTargetBinded() const
        {
            for (const auto& rtv : renderTargetViews)
                if (rtv)
                    return true;

            return false;
        }

    public:
        MultisampleType multisampleType;
        uint32_t width;
        uint32_t height;
        std::array<RenderTargetView::SharedPtr, MaxRenderTargets> renderTargetViews;
        DepthStencilView::SharedPtr depthStencilView;
    };

    class FramebufferDesc::Builder
    {
    public:
        Builder& BindColorTarget(uint32_t index, const RenderTargetView::SharedPtr rtv)
        {
            ASSERT(rtv);

            if (index >= MaxRenderTargets)
            {
                ASSERT_MSG(false, "Wrong binding index.");
                return *this;
            }

            updateMultisampleAndResolution(rtv);
            desc_.renderTargetViews[index] = rtv;

            return *this;
        }

        Builder& BindDepthStecil(const DepthStencilView::SharedPtr dsv)
        {
            ASSERT(dsv);

            updateMultisampleAndResolution(dsv);
            desc_.depthStencilView = dsv;

            return *this;
        }

        operator FramebufferDesc&&()
        {
            return std::move(desc_);
        }

    private:
        bool isBindigsEmpty()
        {
            return (!desc_.IsAnyColorTargetBinded() && desc_.depthStencilView == nullptr);
        }

        GpuResourceDescription getResourceDescription(const GpuResourceView::SharedPtr rv)
        {
            ASSERT(rv);
            const auto gpuResource = rv->GetGpuResource().lock();
            ASSERT(gpuResource);

            return gpuResource->GetDescription();
        }

        void updateMultisampleAndResolution(const GpuResourceView::SharedPtr rv)
        {
            const auto& resourceDesc = getResourceDescription(rv);
            ASSERT(resourceDesc.dimension != GpuResourceDimension::Buffer);

            const auto mipLevel = rv->GetDescription().texture.mipLevel;
            const auto targetWidth = resourceDesc.GetWidth(mipLevel);
            const auto targetHeight = resourceDesc.GetHeight(mipLevel);
            const auto targetMultisampleType = resourceDesc.texture.multisampleType;

            if (isBindigsEmpty())
            {
                desc_.multisampleType = targetMultisampleType;
                desc_.width = targetWidth;
                desc_.height = targetHeight;
            }

            ASSERT(desc_.multisampleType == targetMultisampleType);
            ASSERT(desc_.width == targetWidth);
            ASSERT(desc_.height == targetHeight);
        }

    private:
        FramebufferDesc desc_ = {};
    };

    inline FramebufferDesc::Builder FramebufferDesc::Make()
    {
        return Builder();
    }

    class IFramebuffer
    {
    public:
        virtual ~IFramebuffer() = default;
    };

    class Framebuffer final : public Resource<IFramebuffer, false>
    {
    public:
        using SharedPtr = eastl::shared_ptr<Framebuffer>;

        static constexpr uint32_t MaxRenderTargets = FramebufferDesc::MaxRenderTargets;

        const FramebufferDesc& GetDescription() const { return description_; }

    private:
        static SharedPtr Create(const FramebufferDesc& description)
        {
            return SharedPtr(new Framebuffer(description));
        }

        Framebuffer(const FramebufferDesc& description)
            : Resource(Object::Type::Framebuffer),
              description_(description)
        {
            for (size_t index = 0; index < renderTargetResources_.size(); index++)
            {
                const auto& rtv = description_.renderTargetViews[index];
                renderTargetResources_[index] = rtv ? rtv->GetGpuResource().lock() : nullptr;
            }

            depthStencilResource_ = description_.depthStencilView ? description_.depthStencilView->GetGpuResource().lock() : nullptr;
        }

    private:
        FramebufferDesc description_;

        // Resouce ownership prevents deleting resource while framebuffer alive.
        std::array<GpuResource::SharedPtr, MaxRenderTargets> renderTargetResources_;
        GpuResource::SharedPtr depthStencilResource_;

        friend class Render::DeviceContext;
    };
}