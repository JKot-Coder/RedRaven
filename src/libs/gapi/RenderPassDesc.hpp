#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Limits.hpp"
#if ENABLE_ASSERTS
#include "gapi/GpuResource.hpp"
#include "gapi/Texture.hpp"
#endif

#include "math/VectorMath.hpp"

#include "common/EnumClassOperators.hpp"

namespace RR::GAPI
{
    enum class AttachmentLoadOp : uint8_t
    {
        Load,
        Clear,
        Discard,
    };

    enum class AttachmentStoreOp : uint8_t
    {
        Store,
        Discard,
    };

    struct ColorAttachmentDesc
    {
        const IGpuResourceView* renderTargetView = nullptr;
        GpuResourceFormat format;
        AttachmentLoadOp loadOp;
        AttachmentStoreOp storeOp;
        Vector4 clearColor;
    };

    enum class DepthStencilClearFlags : uint8_t
    {
        None = 0,
        Depth = 1 << 0,
        Stencil = 1 << 1,
        DepthStencil = Depth | Stencil
    };
    ENUM_CLASS_BITWISE_OPS(DepthStencilClearFlags);

    struct DepthStencilAttachmentDesc
    {
        const IGpuResourceView* depthStencilView = nullptr;
        GpuResourceFormat format;
        AttachmentLoadOp loadOp;
        AttachmentStoreOp storeOp;
        DepthStencilClearFlags clearFlags;
        float depthClearValue;
        uint8_t stencilClearValue;
    };

    struct RenderPassDesc
    {
    private:
        struct BuilderImpl;

    public:
        static constexpr uint32_t MaxColorAttachments = MAX_COLOR_ATTACHMENT_COUNT;

        [[nodiscard]] static BuilderImpl Builder();

        uint32_t colorAttachmentCount;
        eastl::array<ColorAttachmentDesc, MaxColorAttachments> colorAttachments;
        DepthStencilAttachmentDesc depthStencilAttachment;
    };

    struct RenderPassDesc::BuilderImpl
    {
        BuilderImpl& ColorAttachment(uint32_t index, const RenderTargetView* renderTargetView, AttachmentLoadOp loadOp = AttachmentLoadOp::Load, Vector4 clearColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f), AttachmentStoreOp storeOp = AttachmentStoreOp::Store)
        {
            ASSERT(renderTargetView);
            ASSERT(index < desc.colorAttachments.size());
            ASSERT(desc.colorAttachments[index].renderTargetView == nullptr);

            const auto& textureDesc = renderTargetView->GetGpuResource()->GetDesc();

            desc.colorAttachments[index] = ColorAttachmentDesc {
                renderTargetView->GetPrivateImpl<IGpuResourceView>(),
                textureDesc.texture.format,
                loadOp, storeOp, clearColor};
            desc.colorAttachmentCount = eastl::max(desc.colorAttachmentCount, index + 1);

#if ENABLE_ASSERTS
            validate(textureDesc);
#endif
            return *this;
        }

        BuilderImpl& DepthStencilAttachment(const DepthStencilView* depthStencilView, AttachmentLoadOp loadOp = AttachmentLoadOp::Load, DepthStencilClearFlags clearFlags = DepthStencilClearFlags::None, float depthClearValue = 0.0f, uint8_t stencilClearValue = 0, AttachmentStoreOp storeOp = AttachmentStoreOp::Store)
        {
            ASSERT(depthStencilView);
            ASSERT((loadOp == AttachmentLoadOp::Clear && clearFlags != DepthStencilClearFlags::None) ||
                   (loadOp != AttachmentLoadOp::Clear && clearFlags == DepthStencilClearFlags::None));

            const auto& textureDesc = depthStencilView->GetGpuResource()->GetDesc();

            desc.depthStencilAttachment = {
                depthStencilView->GetPrivateImpl<IGpuResourceView>(),
                textureDesc.texture.format,
                loadOp, storeOp, clearFlags, depthClearValue, stencilClearValue};

#if ENABLE_ASSERTS
            validate(textureDesc);
#endif
            return *this;
        }

        [[nodiscard]] RenderPassDesc Build() const
        {
            return desc;
        }

#if ENABLE_ASSERTS
        void validate(const GpuResourceDesc& textureDesc)
        {
            if (width == INVALID_SIZE)
            {
                width = textureDesc.texture.width;
                height = textureDesc.texture.height;
                depth = textureDesc.texture.depth;
                dimension = textureDesc.GetDimension();
            }
            else
            {
                ASSERT_MSG(width == textureDesc.GetWidth() && height == textureDesc.GetHeight() && depth == textureDesc.GetDepth(), "attachment size mismatch");
                ASSERT_MSG(dimension == textureDesc.GetDimension(), "attachment dimension mismatch");
            }
        }
#endif

    private:
        RenderPassDesc desc;

#if ENABLE_ASSERTS
        static constexpr uint32_t INVALID_SIZE = 0xFFFFFFFF;
        uint32_t width = INVALID_SIZE, height = INVALID_SIZE, depth = INVALID_SIZE;
        GpuResourceDimension dimension = GpuResourceDimension::Count;
#endif
    };

    inline RenderPassDesc::BuilderImpl RenderPassDesc::Builder()
    {
        return BuilderImpl();
    }
}