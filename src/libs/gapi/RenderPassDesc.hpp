#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Limits.hpp"

#include "math/VectorMath.hpp"

#include "common/EnumClassOperators.hpp"

namespace RR::GAPI
{
    enum class AttachmentLoadOp
    {
        Load,
        Clear,
        Discard,
    };

    enum class AttachmentStoreOp
    {
        Store,
        Discard,
    };

    struct ColorAttachmentDesc
    {
        const RenderTargetView* renderTargetView = nullptr;
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
        const DepthStencilView* depthStencilView = nullptr;
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
        static constexpr uint32_t MaxColorAttachments = MAX_RENDER_TARGETS_COUNT;

        [[nodiscard]] static BuilderImpl Builder();

        eastl::array<ColorAttachmentDesc, MaxColorAttachments> colorAttachments;
        DepthStencilAttachmentDesc depthStencilAttachment;
    };

    struct RenderPassDesc::BuilderImpl
    {
        BuilderImpl& AddColorAttachment(const RenderTargetView* renderTargetView, AttachmentLoadOp loadOp = AttachmentLoadOp::Load, Vector4 clearColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f), AttachmentStoreOp storeOp = AttachmentStoreOp::Store)
        {
            ASSERT(colorAttachmentCount < desc.colorAttachments.size());
            desc.colorAttachments[colorAttachmentCount] = ColorAttachmentDesc {renderTargetView, loadOp, storeOp, clearColor};
            colorAttachmentCount++;
            return *this;
        }

        BuilderImpl& AddDepthStencilAttachment(const DepthStencilView* depthStencilView, AttachmentLoadOp loadOp = AttachmentLoadOp::Load, DepthStencilClearFlags clearFlags = DepthStencilClearFlags::None, float depthClearValue = 0.0f, uint8_t stencilClearValue = 0, AttachmentStoreOp storeOp = AttachmentStoreOp::Store)
        {
            ASSERT((loadOp == AttachmentLoadOp::Clear && clearFlags != DepthStencilClearFlags::None) ||
                   (loadOp != AttachmentLoadOp::Clear && clearFlags == DepthStencilClearFlags::None));

            desc.depthStencilAttachment = {depthStencilView, loadOp, storeOp, clearFlags, depthClearValue, stencilClearValue};
            return *this;
        }

        [[nodiscard]] RenderPassDesc Build() const { return desc; }

    private:
        uint32_t colorAttachmentCount = 0;
        RenderPassDesc desc;
    };

    inline RenderPassDesc::BuilderImpl RenderPassDesc::Builder()
    {
        return BuilderImpl();
    }
}