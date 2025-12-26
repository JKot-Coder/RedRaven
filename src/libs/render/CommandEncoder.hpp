#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/CommandList.hpp"
#include "gapi/commands/Draw.hpp"

#include "math/ForwardDeclarations.hpp"
#include "render/Effect.hpp"
#include "common/NonCopyableMovable.hpp"

namespace RR::Render
{
    class DeviceContext;
    class RenderPassEncoder;

    class CommandEncoder
    {
    public:
        using UniquePtr = eastl::unique_ptr<CommandEncoder>;

    public:
        RenderPassEncoder BeginRenderPass(const GAPI::RenderPassDesc& renderPass);

        void Finish()
        {
            ASSERT_MSG(state == State::Open, "CommandEncoder was not started");
            state = State::Closed;
        };

    private:
        enum class State : uint8_t
        {
            Open,
            Closed,
            RenderPass,
        };

    private:
        friend class RR::Render::DeviceContext;
        friend class PassEncoderBase;
        friend class RenderPassEncoder;

        explicit CommandEncoder(GAPI::CommandList&& commandlist) : commandList(eastl::move(commandlist)) { }
        GAPI::CommandList& GetCommandList() { return commandList; }

        static UniquePtr Create(GAPI::CommandList&& commandlist)
        {
            return UniquePtr(new CommandEncoder(eastl::move(commandlist)));
        }

        void EndRenderPass()
        {
            ASSERT_MSG(state == State::RenderPass, "RenderPass was not started");
            state = State::Open;
        }

        void Reset()
        {
            ASSERT_MSG(state == State::Closed, "CommandEncoder was not finished");
            state = State::Open;
        }

    private:
        State state = State::Open;
        GAPI::CommandList commandList;
    };

    class PassEncoderBase : public Common::NonCopyable
    {
    protected:
        PassEncoderBase(CommandEncoder& commandContext) : commandContext(&commandContext) { }

        ~PassEncoderBase()
        {
            ASSERT_MSG(!commandContext, "PassEncoder was not ended");
        }

        GAPI::CommandList& GetCommandList()
        {
            ASSERT(commandContext);
            return commandContext->GetCommandList();
        }

        CommandEncoder& GetCommandEncoder()
        {
            ASSERT(commandContext);
            return *commandContext;
        }

        void reset()
        {
            ASSERT(commandContext);
            commandContext = nullptr;
        }

    private:
        CommandEncoder* commandContext = nullptr;
    };

    class RenderPassEncoder final : private PassEncoderBase
    {
    public:
        using Base = PassEncoderBase;

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
        void Draw(Effect* effect, GAPI::PrimitiveTopology topology, uint32_t startVertex, uint32_t vertexCount, uint32_t instanceCount = 0);
        void DrawIndexed(Effect* effect, GAPI::PrimitiveTopology topology, uint32_t startIndex, uint32_t indexCount, uint32_t instanceCount = 0);
        void End();

    private:
        friend class Render::CommandEncoder;

        RenderPassEncoder(CommandEncoder& commandContext, const GAPI::RenderPassDesc& renderPass) : PassEncoderBase(commandContext)
        {
            setRenderPass(renderPass);
        }

        static RenderPassEncoder Create(CommandEncoder& commandContext, const GAPI::RenderPassDesc& renderPass)
        {
            return RenderPassEncoder(commandContext, renderPass);
        }

        void setRenderPass(const GAPI::RenderPassDesc& renderPass);
        GAPI::Commands::GeometryLayout& flushLayout() { return geometryManager.flush(GetCommandList()); }

    private:
        GraphicsParams graphicsParams;
        GeometryManager geometryManager;
    };

    inline RenderPassEncoder CommandEncoder::BeginRenderPass(const GAPI::RenderPassDesc& renderPass)
    {
        ASSERT(state == State::Open);
        state = State::RenderPass;
        return RenderPassEncoder::Create(*this, renderPass);
    }
}