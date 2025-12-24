#include "CommandListImpl.hpp"

#include "gapi/commands/SetRenderPass.hpp"

#include "TextureViewImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

namespace RR::GAPI::WebGPU
{
    namespace
    {
        struct CommandCompileContext
        {
            wgpu::CommandEncoder encoder;
        };

        wgpu::LoadOp getWGPULoadOp(GAPI::AttachmentLoadOp loadOp)
        {
            switch (loadOp)
            {
                case GAPI::AttachmentLoadOp::Load: return wgpu::LoadOp::Load;
                case GAPI::AttachmentLoadOp::Clear:
                case GAPI::AttachmentLoadOp::Discard: return wgpu::LoadOp::Clear;
                default:
                    ASSERT_MSG(false, "Unsupported load op");
                    return wgpu::LoadOp::Undefined;
            }
        }

        wgpu::StoreOp getWGPUStoreOp(GAPI::AttachmentStoreOp storeOp)
        {
            switch (storeOp)
            {
                case GAPI::AttachmentStoreOp::Store: return wgpu::StoreOp::Store;
                case GAPI::AttachmentStoreOp::Discard: return wgpu::StoreOp::Discard;
                default:
                    ASSERT_MSG(false, "Unsupported store op");
                    return wgpu::StoreOp::Undefined;
            }
        }

        wgpu::Color getWGPUColor(const Vector4& clearColor)
        {
            return wgpu::Color{ clearColor.x, clearColor.y, clearColor.z, clearColor.w };
        }

        void compileCommand(const Commands::SetRenderPass& command, CommandCompileContext& ctx)
        {
            eastl::array<wgpu::RenderPassColorAttachment, MAX_RENDER_TARGETS_COUNT> colorAttachments;

            for (uint32_t i = 0; i < command.desc.colorAttachmentCount; i++)
            {
                wgpu::RenderPassColorAttachment colorAttachment;
                colorAttachment.setDefault();
                colorAttachment.view = command.desc.colorAttachments[i].renderTargetView->GetPrivateImpl<TextureViewImpl>()->GetTextureView();
                colorAttachment.loadOp = getWGPULoadOp(command.desc.colorAttachments[i].loadOp);
                colorAttachment.storeOp = getWGPUStoreOp(command.desc.colorAttachments[i].storeOp);
                colorAttachment.clearValue = getWGPUColor(command.desc.colorAttachments[i].clearColor);

                colorAttachments[i] = eastl::move(colorAttachment);
            }

            wgpu::RenderPassDescriptor renderPassDescriptor;
            renderPassDescriptor.setDefault();
            renderPassDescriptor.colorAttachmentCount = command.desc.colorAttachmentCount;
            renderPassDescriptor.colorAttachments = colorAttachments.data();

            if (command.desc.depthStencilAttachment.depthStencilView)
            {
                wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
                depthStencilAttachment.setDefault();
                // TODO: FIX separate depth and stencil load and store ops
                depthStencilAttachment.view = command.desc.depthStencilAttachment.depthStencilView->GetPrivateImpl<TextureViewImpl>()->GetTextureView();
                depthStencilAttachment.depthLoadOp = getWGPULoadOp(command.desc.depthStencilAttachment.loadOp);
                depthStencilAttachment.depthStoreOp = getWGPUStoreOp(command.desc.depthStencilAttachment.storeOp);
                depthStencilAttachment.depthClearValue = command.desc.depthStencilAttachment.depthClearValue;
                depthStencilAttachment.stencilLoadOp = getWGPULoadOp(command.desc.depthStencilAttachment.loadOp);
                depthStencilAttachment.stencilStoreOp = getWGPUStoreOp(command.desc.depthStencilAttachment.storeOp);
                depthStencilAttachment.stencilClearValue = command.desc.depthStencilAttachment.stencilClearValue;

                renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
            }

            auto renderPassEncoder = ctx.encoder.beginRenderPass(renderPassDescriptor);
            renderPassEncoder.end();
            renderPassEncoder.release();
        }
    }
    CommandListImpl::~CommandListImpl() { }

    void CommandListImpl::Init(wgpu::Device device)
    {
        UNUSED(device);
    }

    void CommandListImpl::Compile(wgpu::Device device,GAPI::CommandList2& commandList)
    {
        ASSERT(!commandBuffer);

        wgpu::CommandEncoderDescriptor commandEncoderDescriptor;
        commandEncoderDescriptor.setDefault();

        auto commandEncoder = device.createCommandEncoder(commandEncoderDescriptor);
        CommandCompileContext ctx{ commandEncoder };

        ASSERT(commandList.size() != 0);

        for (const auto* command : commandList)
        {
            switch (command->type)
            {
            case Command::Type::SetRenderPass:
                compileCommand(static_cast<const Commands::SetRenderPass&>(*command), ctx);
                break;

            default:
                ASSERT_MSG(false, "Unsupported command type");
                break;
            }
        }

        commandBuffer = commandEncoder.finish();
        commandEncoder.release();
    }
}
