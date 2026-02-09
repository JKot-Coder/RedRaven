#include "CommandListImpl.hpp"

#include "gapi/commands/RenderPass.hpp"
#include "gapi/commands/Draw.hpp"

#include "gapi/Buffer.hpp"

#include "BufferImpl.hpp"
#include "PipelineStateImpl.hpp"
#include "TextureViewImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

namespace RR::GAPI::WebGPU
{
    namespace
    {
        struct CommandCompileContext
        {
            wgpu::CommandEncoder encoder;
            wgpu::RenderPassEncoder renderPassEncoder;
        };

        wgpu::LoadOp getWGPULoadOp(GAPI::AttachmentLoadOp loadOp)
        {
            switch (loadOp)
            {
                case GAPI::AttachmentLoadOp::Discard:
                case GAPI::AttachmentLoadOp::Load: return wgpu::LoadOp::Load;
                case GAPI::AttachmentLoadOp::Clear: return wgpu::LoadOp::Clear;
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

        void compileCommand(const Commands::BeginRenderPass& command, CommandCompileContext& ctx)
        {
            eastl::array<wgpu::RenderPassColorAttachment, MAX_COLOR_ATTACHMENT_COUNT> colorAttachments;

            for (uint32_t i = 0; i < command.desc.colorAttachmentCount; i++)
            {
                wgpu::RenderPassColorAttachment colorAttachment;
                colorAttachment.setDefault();
                colorAttachment.view = static_cast<const TextureViewImpl*>(command.desc.colorAttachments[i].renderTargetView)->GetTextureView();
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
                depthStencilAttachment.view = static_cast<const TextureViewImpl*>(command.desc.depthStencilAttachment.depthStencilView)->GetTextureView();
                depthStencilAttachment.depthLoadOp = getWGPULoadOp(command.desc.depthStencilAttachment.loadOp);
                depthStencilAttachment.depthStoreOp = getWGPUStoreOp(command.desc.depthStencilAttachment.storeOp);
                depthStencilAttachment.depthClearValue = command.desc.depthStencilAttachment.depthClearValue;
                depthStencilAttachment.stencilLoadOp = getWGPULoadOp(command.desc.depthStencilAttachment.loadOp);
                depthStencilAttachment.stencilStoreOp = getWGPUStoreOp(command.desc.depthStencilAttachment.storeOp);
                depthStencilAttachment.stencilClearValue = command.desc.depthStencilAttachment.stencilClearValue;

                renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
            }

            ctx.renderPassEncoder = ctx.encoder.beginRenderPass(renderPassDescriptor);
        }

        void compileCommand(const Commands::EndRenderPass&, CommandCompileContext& ctx)
        {
            ctx.renderPassEncoder.end();
            ctx.renderPassEncoder.release();
        }

        // ---------------------------------------------------------------------------------------------
        // Draw commands
        // ---------------------------------------------------------------------------------------------

        template <typename T>
        void compileCommand(const T& command, bool indexed, CommandCompileContext& ctx)
        {
            const auto* pso = static_cast<const PipelineStateImpl*>(command.psoImpl);
            ASSERT(pso);
            ASSERT(command.geometryLayout);

            ctx.renderPassEncoder.setPipeline(pso->GetRenderPipeline());
            ASSERT(command.geometryLayout->vertexBindings.size() <= MAX_VERTEX_BUFFERS);
            const size_t vertexBindingsCount = command.geometryLayout->vertexBindings.size();

            // Set vertex buffers
            for (size_t i = 0; i < vertexBindingsCount; i++)
            {
                const auto& vertexBinding = command.geometryLayout->vertexBindings[i];
                if (vertexBinding.vertexBuffer)
                {
                    const auto* bufferImpl = static_cast<const BufferImpl*>(vertexBinding.vertexBuffer);
                    ctx.renderPassEncoder.setVertexBuffer(
                        static_cast<uint32_t>(i),
                        bufferImpl->GetBuffer(),
                        vertexBinding.vertexBufferOffset,
                        bufferImpl->GetSize() - vertexBinding.vertexBufferOffset);
                }
            }

            if (indexed)
            {
                const auto* indexBuffer = static_cast<const BufferImpl*>(command.geometryLayout->indexBuffer);
                ASSERT(indexBuffer);

                const wgpu::IndexFormat indexFormat = indexBuffer->GetIndexFormat();
                if (indexFormat != wgpu::IndexFormat::Uint16 && indexFormat != wgpu::IndexFormat::Uint32)
                {
                    ASSERT_MSG(false, "Unknown index buffer format");
                    return;
                }

                ctx.renderPassEncoder.setIndexBuffer(indexBuffer->GetBuffer(), indexFormat, 0, indexBuffer->GetSize() - 0);

                const uint32_t instanceCount = Max(command.attribs.instanceCount, 1u);
                ctx.renderPassEncoder.drawIndexed(
                    command.attribs.vertexCount,
                    instanceCount,
                    command.attribs.startLocation,
                    0, // baseVertex
                    0  // firstInstance
                );
            }
            else
            {
                const uint32_t instanceCount = Max(command.attribs.instanceCount, 1u);
                ctx.renderPassEncoder.draw(
                    command.attribs.vertexCount,
                    instanceCount,
                    command.attribs.startLocation,
                    0  // firstInstance
                );
            }
        }

        void compileCommand(const Commands::Draw& command, CommandCompileContext& ctx)
        {
            compileCommand(command, false, ctx);
        }

        void compileCommand(const Commands::DrawIndexed& command, CommandCompileContext& ctx)
        {
            compileCommand(command, true, ctx);
        }
    }
    CommandListImpl::~CommandListImpl() { }

    void CommandListImpl::Init(wgpu::Device device)
    {
        UNUSED(device);
    }

    void CommandListImpl::Compile(wgpu::Device device, GAPI::CommandList& commandList)
    {
        ASSERT(!commandBuffer);

        wgpu::CommandEncoderDescriptor commandEncoderDescriptor;
        commandEncoderDescriptor.setDefault();

        auto commandEncoder = device.createCommandEncoder(commandEncoderDescriptor);
        CommandCompileContext ctx{ commandEncoder, nullptr };

        for (const auto* command : commandList)
        {
            switch (command->type)
            {
            case Command::Type::BeginRenderPass:
                compileCommand(static_cast<const Commands::BeginRenderPass&>(*command), ctx);
                break;

            case Command::Type::EndRenderPass:
                compileCommand(static_cast<const Commands::EndRenderPass&>(*command), ctx);
                break;

            case Command::Type::Draw:
                compileCommand(static_cast<const Commands::Draw&>(*command), ctx);
                break;

            case Command::Type::DrawIndexed:
                compileCommand(static_cast<const Commands::DrawIndexed&>(*command), ctx);
                break;

            default:
                ASSERT_MSG(false, "Unsupported command type");
                break;
            }
        }

        commandBuffer = commandEncoder.finish();
        commandEncoder.release();

        commandList.clear();
    }
}
