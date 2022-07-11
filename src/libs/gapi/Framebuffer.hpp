#pragma once

#include "gapi/GpuResource.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/Texture.hpp"

// TODO temporary
#include <any>

namespace RR::GAPI
{
    struct FramebufferDescription
    {
    private:
        class Builder;

    public:
        static constexpr uint32_t MaxRenderTargets = 8;

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

    class FramebufferDescription::Builder
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

        operator FramebufferDescription&&()
        {
            return std::move(desc_);
        }

    private:
        bool isBindigsEmpty()
        {
            return (!desc_.IsAnyColorTargetBinded() && desc_.depthStencilView == nullptr);
        }

        TextureDescription getTextureDescription(const GpuResourceView::SharedPtr rv)
        {
            ASSERT(rv);
            const auto gpuResource = rv->GetGpuResource().lock();
            ASSERT(gpuResource);

            return gpuResource->GetTyped<Texture>()->GetDescription();
        }

        void updateMultisampleAndResolution(const GpuResourceView::SharedPtr rv)
        {
            const auto& textureDesc = getTextureDescription(rv);

            const auto mipLevel = rv->GetDescription().texture.mipLevel;
            const auto textureWidth = textureDesc.GetWidth(mipLevel);
            const auto textureHeight = textureDesc.GetHeight(mipLevel);

            if (isBindigsEmpty())
            {
                desc_.multisampleType = textureDesc.multisampleType;
                desc_.width = textureWidth;
                desc_.height = textureHeight;
            }

            ASSERT(desc_.multisampleType == textureDesc.multisampleType);
            ASSERT(desc_.width == textureWidth);
            ASSERT(desc_.height == textureHeight);
        }

    private:
        FramebufferDescription desc_ = {};
    };

    inline FramebufferDescription::Builder FramebufferDescription::Make()
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
        using SharedPtr = std::shared_ptr<Framebuffer>;

        static constexpr uint32_t MaxRenderTargets = FramebufferDescription::MaxRenderTargets;

        const FramebufferDescription& GetDescription() const { return description_; }

    private:
        static SharedPtr Create(const FramebufferDescription& description)
        {
            return SharedPtr(new Framebuffer(description));
        }

        Framebuffer(const FramebufferDescription& description)
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
        FramebufferDescription description_;

        // Resouce ownership prevents deleting resource while framebuffer alive.
        std::array<GpuResource::SharedPtr, MaxRenderTargets> renderTargetResources_;
        GpuResource::SharedPtr depthStencilResource_;

        friend class Render::DeviceContext;
    };
}