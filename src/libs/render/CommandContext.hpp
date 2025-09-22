#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/CommandList2.hpp"

#include "math/ForwardDeclarations.hpp"
#include "render/Effect.hpp"

namespace RR::Render
{
    class DeviceContext;

    class CommandContext
    {
    protected:
        GAPI::CommandList2& GetCommandList() { return commandList; }

        friend class DeviceContext;

    protected:
        explicit CommandContext(GAPI::CommandList2&& commandlist) : commandList(eastl::move(commandlist)) { }
        GAPI::CommandList2 commandList;
    };

    class CopyCommandContext : public CommandContext
    {
    public:
        CopyCommandContext(GAPI::CommandList2&& commandlist) : CommandContext(eastl::move(commandlist)) { }
    };

    class ComputeCommandContext : public CopyCommandContext
    {
    public:
        ComputeCommandContext(GAPI::CommandList2&& commandlist) : CopyCommandContext(eastl::move(commandlist)) { }
    };

    class GraphicsCommandContext final : public ComputeCommandContext
    {
    public:
        using UniquePtr = eastl::unique_ptr<GraphicsCommandContext>;

    public:
        void SetVertexBuffers(uint32_t slot, const GAPI::Buffer& buffer, uint32_t offset = 0);
        void SetIndexBuffer(const GAPI::Buffer* buffer);
        void SetRenderPass(const GAPI::RenderPassDesc& renderPass);
        void Draw(Effect* effect, GAPI::PrimitiveTopology topology, uint32_t startVertex, uint32_t vertexCount, uint32_t instanceCount = 0);
        void DrawIndexed(Effect* effect, GAPI::PrimitiveTopology topology, uint32_t startIndex, uint32_t indexCount, uint32_t instanceCount = 0);

    private:
        friend class Render::DeviceContext;

        GraphicsCommandContext(GAPI::CommandList2&& commandlist) : ComputeCommandContext(eastl::move(commandlist)) { }
        static UniquePtr Create(GAPI::CommandList2&& commandlist)
        {
            return eastl::unique_ptr<GraphicsCommandContext>(new GraphicsCommandContext(eastl::move(commandlist)));
        }

        void reset()
        {
            // We reset only internal state here. Command list still in use until submit.
            graphicsParams.Reset();
        }

    private:
        GraphicsParams graphicsParams;
        const GAPI::Buffer* indexBuffer = nullptr;
    };
}