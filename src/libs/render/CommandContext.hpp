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
    public:
        GAPI::CommandList2& GetCommandList() { return commandList; }

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

    class GraphicsCommandContext : public ComputeCommandContext
    {
    public:
        using UniquePtr = eastl::unique_ptr<GraphicsCommandContext>;

    public:
        void SetRenderPass(const GAPI::RenderPassDesc& renderPass);
        void Draw(Effect* effect, GAPI::PrimitiveTopology topology, uint32_t startVertex, uint32_t vertexCount, uint32_t instanceCount = 0);

    private:
        friend class Render::DeviceContext;

        GraphicsCommandContext(GAPI::CommandList2&& commandlist) : ComputeCommandContext(eastl::move(commandlist)) { }
        static UniquePtr Create(GAPI::CommandList2&& commandlist)
        {
            return eastl::unique_ptr<GraphicsCommandContext>(new GraphicsCommandContext(eastl::move(commandlist)));
        }

    private:
        GraphicsParams graphicsParams;
    };
}