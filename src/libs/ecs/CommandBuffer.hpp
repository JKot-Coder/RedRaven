#pragma once

#include "common/ChunkAllocator.hpp"

namespace RR::Ecs
{
    struct World;
    enum class CommandType : uint8_t;

    struct Command
    {
        CommandType type;
        virtual ~Command() = default;
    };

    struct CommandBuffer final
    {
        private:
            static constexpr size_t InitialCommandQueueSize = 1024*1024;

        public:
            CommandBuffer() : allocator(InitialCommandQueueSize) { }

            void ProcessCommands(World& world);

        private:
            Common::ChunkAllocator allocator;
            eastl::vector<Command*> commands;
    };
}
