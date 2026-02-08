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

            desc.colorAttachments[index] = ColorAttachmentDesc {renderTargetView, loadOp, storeOp, clearColor};
            desc.colorAttachmentCount = eastl::max(desc.colorAttachmentCount, index + 1);
            return *this;
        }

        BuilderImpl& DepthStencilAttachment(const DepthStencilView* depthStencilView, AttachmentLoadOp loadOp = AttachmentLoadOp::Load, DepthStencilClearFlags clearFlags = DepthStencilClearFlags::None, float depthClearValue = 0.0f, uint8_t stencilClearValue = 0, AttachmentStoreOp storeOp = AttachmentStoreOp::Store)
        {
            ASSERT(depthStencilView);
            ASSERT((loadOp == AttachmentLoadOp::Clear && clearFlags != DepthStencilClearFlags::None) ||
                   (loadOp != AttachmentLoadOp::Clear && clearFlags == DepthStencilClearFlags::None));

            desc.depthStencilAttachment = {depthStencilView, loadOp, storeOp, clearFlags, depthClearValue, stencilClearValue};
            return *this;
        }

        [[nodiscard]] RenderPassDesc Build() const
        {
#if ENABLE_ASSERTS
            AssertValid();
#endif
            return desc;
        }

#if ENABLE_ASSERTS
        void AssertValid() const
        {
            ASSERT(desc.colorAttachmentCount < desc.colorAttachments.size());

            static constexpr uint32_t INVALID_SIZE = 0xFFFFFFFF;
            uint32_t width = INVALID_SIZE, height = INVALID_SIZE, depth = INVALID_SIZE;
            GpuResourceDimension dimension = GpuResourceDimension::Count;
            for (uint32_t i = 0; i < desc.colorAttachmentCount; i++)
            {
                const auto* renderTargetView = desc.colorAttachments[i].renderTargetView;
                if (renderTargetView == nullptr)
                    continue;

                auto gpuResource = renderTargetView->GetGpuResource();
                auto texture = gpuResource->GetTyped<Texture>();
                auto description = texture->GetDesc();

                if (dimension == GpuResourceDimension::Count)
                {
                    dimension = description.GetDimension();
                }
                else
                {
                    ASSERT_MSG(dimension == description.GetDimension(), "Color attachment dimension mismatch");
                }

                if (width == INVALID_SIZE)
                {
                    width = description.GetWidth();
                    height = description.GetHeight();
                    depth = description.GetDepth();
                }
                else
                {
                    ASSERT_MSG(width == description.GetWidth() && height == description.GetHeight() && depth == description.GetDepth(), "Color attachment size mismatch");
                }
            }

            const auto* depthStencilView = desc.depthStencilAttachment.depthStencilView;
            if (depthStencilView)
            {
                auto gpuResource = depthStencilView->GetGpuResource();
                auto texture = gpuResource->GetTyped<Texture>();
                auto description = texture->GetDesc();

                if (width != INVALID_SIZE)
                    ASSERT_MSG(width == description.GetWidth() && height == description.GetHeight() && depth == description.GetDepth(), "Color attachment and DepthStencil attachment size mismatch");

                if (dimension != GpuResourceDimension::Count)
                    ASSERT_MSG(dimension == description.GetDimension(), "DepthStencil attachment dimension mismatch");
            }
        }
#endif

    private:
        RenderPassDesc desc;
    };

    inline RenderPassDesc::BuilderImpl RenderPassDesc::Builder()
    {
        return BuilderImpl();
    }
}