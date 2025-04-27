#include "CommandBuffer.hpp"

#include "ecs/World.hpp"
#include "ecs/ComponentTraits.hpp"

namespace RR::Ecs
{
    struct CommmandProcessors
    {
        static void process(MutateEntityCommand& command, World& world)
        {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)
            Archetype* archetype = nullptr;
            ArchetypeEntityIndex index = {};
            UNUSED(world.ResolveEntityArhetype(command.entityId, archetype, index));

            world.mutateEntity(command.entityId, archetype, index, command.removedComponents, command.addedComponents, [&](Archetype& archetype, ArchetypeEntityIndex entityIndex) {
                ASSERT(command.addedComponents.size() == command.components.size());

                auto component = command.addedComponents.begin();
                for (void* componentPtr : command.components)
                {
                    archetype.MoveComponentFrom(entityIndex, archetype.GetComponentIndex(*component), componentPtr);
                    ++component;
                }
            });
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
    };

    void CommandBuffer::ProcessCommands(World& world)
    {
        if (commands.empty())
            return;

#define PROCESS_COMMAND(commandType)                                                      \
    case CommandType::commandType:                                                        \
        CommmandProcessors::process(*static_cast<commandType##Command*>(command), world); \
        break;

        for (auto command : commands)
        {
            switch (command->type)
            {
                PROCESS_COMMAND(MutateEntity)
                //PROCESS_COMMAND(DestroyEntity)
            // PROCESS_COMMAND(AddComponent)
                    //PROCESS_COMMAND(RemoveComponent)
            default:
                ASSERT_MSG(false, "Unknown command type");
            }
        }

        #undef PROCESS_COMMAND

        allocator.reset();
        commands.clear();
    }
}

