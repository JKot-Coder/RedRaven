#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/CommandList2.hpp"

#include "math/ForwardDeclarations.hpp"

namespace RR::Render
{
    class DeviceContext;
}

namespace RR::GAPI
{
    enum class PrimitiveTopology : uint8_t;

    class CommandContext
    {
    public:
        CommandList2& GetCommandList() { return commandList; }

    protected:
        explicit CommandContext(CommandList2&& commandlist ) : commandList(eastl::move(commandlist)) { }
        CommandList2 commandList;
    };

    class CopyCommandContext: public CommandContext
    {
    public:
        CopyCommandContext(CommandList2&& commandlist) : CommandContext(eastl::move(commandlist)) { }
    };

    class ComputeCommandContext  : public CopyCommandContext
    {
    public:
        ComputeCommandContext(CommandList2&& commandlist) : CopyCommandContext(eastl::move(commandlist)) { }
    };

    class GraphicsCommandContext : public  ComputeCommandContext
    {
    public:
        using UniquePtr = eastl::unique_ptr<GraphicsCommandContext>;

    public:
        void SetFramebuffer(Framebuffer* framebuffer);
        void SetPipelineState(GraphicPipelineState* pso);
        void ClearRenderTargetView(const RenderTargetView* renderTargetView, const Vector4& color);
        void ClearDepthStencilView(const DepthStencilView* depthStencilView, float clearValue);
        void Draw(PrimitiveTopology topology, uint32_t startVertex, uint32_t vertexCount, uint32_t instanceCount = 0);

    private:
        friend class Render::DeviceContext;

        GraphicsCommandContext(CommandList2&& commandlist) : ComputeCommandContext(eastl::move(commandlist)) { }
        static UniquePtr Create(CommandList2&& commandlist)
        {
            return eastl::unique_ptr<GraphicsCommandContext>(new GraphicsCommandContext(eastl::move(commandlist)));
        }

    private:
        Framebuffer* framebuffer = nullptr;
        GraphicPipelineState* pso = nullptr; // TEMPORATY. INVALID PipelineStateCould be destroyed..............
    };
}