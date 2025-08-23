#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"
#include "gapi/commands/Command.hpp"

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
}