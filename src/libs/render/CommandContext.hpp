#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/CommandList.hpp"
#include "gapi/commands/Draw.hpp"

#include "math/ForwardDeclarations.hpp"
#include "render/Effect.hpp"

namespace RR::Render
{
    class DeviceContext;

    class CommandContext
    {
    public:
        using UniquePtr = eastl::unique_ptr<CommandContext>;

    private:
        friend class DeviceContext;
        friend class PassEncoderBase;

        explicit CommandContext(GAPI::CommandList&& commandlist) : commandList(eastl::move(commandlist)) { }

        GAPI::CommandList& GetCommandList() { return commandList; }

        static UniquePtr Create(GAPI::CommandList&& commandlist)
        {
            return UniquePtr(new CommandContext(eastl::move(commandlist)));
        }

    private:
        GAPI::CommandList commandList;
    };

    class PassEncoderBase
    {
    protected:
        PassEncoderBase(CommandContext& commandContext) : commandContext(&commandContext) { }
        GAPI::CommandList& GetCommandList() { return commandContext->GetCommandList(); }

        CommandContext* commandContext = nullptr;
    };

    class RenderPassEncoder final : private PassEncoderBase
    {
    public:
        using UniquePtr = eastl::unique_ptr<RenderPassEncoder>;

    private:
        struct GeometryManager
        {
            GAPI::Commands::GeometryLayout& flush(GAPI::CommandList& commandList);

            void SetVertexBuffer(uint32_t slot, const GAPI::Buffer& buffer, uint32_t offset);
            void SetIndexBuffer(const GAPI::Buffer* buffer);
            void Reset()
            {
                indexBuffer = nullptr;
                vertexBindings.clear();
                currentLayout = nullptr;
                dirty = false;
            }

            eastl::fixed_vector<GAPI::Commands::VertexBinding, 8> vertexBindings;
            const GAPI::Buffer* indexBuffer = nullptr;
            GAPI::Commands::GeometryLayout* currentLayout = nullptr;
            bool dirty = false;
        };

    public:
        void SetVertexLayout(const GAPI::VertexLayout* layout) { graphicsParams.SetVertexLayout(layout); }
        void SetVertexBuffer(uint32_t slot, const GAPI::Buffer& buffer, uint32_t offset = 0) { geometryManager.SetVertexBuffer(slot, buffer, offset); }
        void SetIndexBuffer(const GAPI::Buffer* buffer) { geometryManager.SetIndexBuffer(buffer); }
        void SetRenderPass(const GAPI::RenderPassDesc& renderPass);
        void Draw(Effect* effect, GAPI::PrimitiveTopology topology, uint32_t startVertex, uint32_t vertexCount, uint32_t instanceCount = 0);
        void DrawIndexed(Effect* effect, GAPI::PrimitiveTopology topology, uint32_t startIndex, uint32_t indexCount, uint32_t instanceCount = 0);

    private:
        friend class Render::DeviceContext;

        RenderPassEncoder(CommandContext& commandContext) : PassEncoderBase(commandContext) { }
        static UniquePtr Create(CommandContext& commandContext)
        {
            return eastl::unique_ptr<RenderPassEncoder>(new RenderPassEncoder(commandContext));
        }

        GAPI::Commands::GeometryLayout& flushLayout() { return geometryManager.flush(GetCommandList()); }

        void reset()
        {
            // We reset only internal state here. Command list still in use until submit.
            graphicsParams.Reset();
            geometryManager.Reset();
        }

    private:
        GraphicsParams graphicsParams;
        GeometryManager geometryManager;
    };
}