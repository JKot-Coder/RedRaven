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

    class ICommandContext
    {
    public:
        virtual ~ICommandContext() = default;
    };

    class CommandList2
    {
    public:
        CommandList2(size_t initialCommandCapacity = 100, size_t bufferCapacity = 128 * 1024)
            : allocator(bufferCapacity)
        {
            commands.reserve(initialCommandCapacity);
        }

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
        eastl::vector<Command*> commands;
        Common::ChunkAllocator allocator;
    };

    class CommandContext : public Resource<ICommandContext>
    {
    public:
        using UniquePtr = eastl::unique_ptr<CommandContext>;

        enum class Type
        {
            Graphics,
            Compute,
            Copy,
        };

    public:
        CommandList2& GetCommandList() { return commandList; }

    protected:
        explicit CommandContext(Type type, const std::string& name) : Resource(Resource::Type::CommandList, name), type(type), commandList(100, 128 * 1024) { }

    protected:
        Type type;
        CommandList2 commandList;
    };

    struct CommandContextMixin
    {
    protected:
        CommandContextMixin(CommandList2* commandList) : commandListPtr(commandList) { }
        CommandList2& GetCommandList()
        {
            ASSERT(commandListPtr);
            return *commandListPtr;
        }

    private:
        CommandList2* commandListPtr = nullptr;
    };

    struct CopyCommandContextMixin : virtual CommandContextMixin
    {
    public:
        CopyCommandContextMixin() : CommandContextMixin(nullptr) { }
    };

    struct ComputeCommandContextMixin : virtual CommandContextMixin
    {
    public:
        ComputeCommandContextMixin() : CommandContextMixin(nullptr) { }
    };

    struct GraphicsOperationsMixin : virtual CommandContextMixin
    {
    public:
        GraphicsOperationsMixin() : CommandContextMixin(nullptr) { }

        void SetFramebuffer(Framebuffer* framebuffer);
        void SetPipelineState(GraphicPipelineState* pso);
        void ClearRenderTargetView(const RenderTargetView* renderTargetView, const Vector4& color);
    private:
        Framebuffer* framebuffer = nullptr;
        GraphicPipelineState* pso = nullptr; // TEMPORATY. INVALID PipelineStateCould be destroyed..............
    };

    class CopyCommandContext final : public CommandContext,
                                     public CopyCommandContextMixin
    {
    public:
        using UniquePtr = eastl::unique_ptr<CopyCommandContext>;

    private:
        friend class Render::DeviceContext;

        explicit CopyCommandContext(const std::string& name)
            : CommandContextMixin(&commandList), CommandContext(Type::Copy, name), CopyCommandContextMixin()
        {
        }
    };

    class ComputeCommandContext final : public CommandContext,
                                        public CopyCommandContextMixin,
                                        public ComputeCommandContextMixin
    {
    public:
        using UniquePtr = eastl::unique_ptr<ComputeCommandContext>;

    private:
        friend class Render::DeviceContext;

        explicit ComputeCommandContext(const std::string& name)
            : CommandContextMixin(&commandList), CommandContext(Type::Compute, name), CopyCommandContextMixin(), ComputeCommandContextMixin()
        {
        }
    };

    class GraphicsCommandContext final : public CommandContext,
                                         public GraphicsOperationsMixin,
                                         public CopyCommandContextMixin,
                                         public ComputeCommandContextMixin
    {
    public:
        using UniquePtr = eastl::unique_ptr<GraphicsCommandContext>;

    private:
        friend class Render::DeviceContext;

        explicit GraphicsCommandContext(const std::string& name)
            : CommandContextMixin(&commandList), CommandContext(Type::Graphics, name), GraphicsOperationsMixin(), CopyCommandContextMixin(), ComputeCommandContextMixin()
        {
        }

        static GraphicsCommandContext::UniquePtr Create(const std::string& name)
        {
            return eastl::unique_ptr<GraphicsCommandContext>(new GraphicsCommandContext(name));
        }
    };
}