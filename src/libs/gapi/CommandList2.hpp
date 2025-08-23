#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"
#include "gapi/commands/Command.hpp"

#include "math/ForwardDeclarations.hpp"

#include "common/ChunkAllocator.hpp"

namespace RR::Render
{
    class DeviceContext;
}

namespace RR::GAPI
{
    enum class PrimitiveTopology : uint8_t;

    class ICommandList
    {
    public:
        virtual ~ICommandList() = default;
    };

    class CommandList2 final : public Resource<ICommandList>
    {
    public:
        template <typename CommandType, typename... Args>
        void emplaceCommand(Args&&... params)
        {
            static_assert(std::is_base_of<Command, CommandType>::value);
            static_assert(std::is_trivially_move_constructible<CommandType>::value);

            void* commandStorage = allocator.create<CommandType>(eastl::forward<Args>(params)...);
            commands.push_back(static_cast<Command*>(commandStorage));
        }

        auto begin() { return commands.begin(); }
        auto end() { return commands.end(); }
        auto begin() const { return commands.begin(); }
        auto end() const { return commands.end(); }
        size_t size() const { return commands.size(); }
        void clear()
        {
            commands.clear();
            allocator.reset();
        }

    private:
        friend class Render::DeviceContext;

        CommandList2(const std::string& name, size_t initialCommandCapacity = 100, size_t bufferCapacity = 128 * 1024)
            :  Resource(Type::CommandList, name), allocator(bufferCapacity)
        {
            commands.reserve(initialCommandCapacity);
        }

    private:
        eastl::vector<Command*> commands;
        Common::ChunkAllocator allocator;
    };

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