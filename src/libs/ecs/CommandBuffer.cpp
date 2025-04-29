#include "CommandBuffer.hpp"

#include "ecs/World.hpp"
#include "ecs/ComponentTraits.hpp"

namespace RR::Ecs
{
    struct DestroyEntityCommand final : public Command
    {
        DestroyEntityCommand(EntityId entityId)
            : Command(CommandType::DestroyEntity),
              entityId(entityId) {};
        EntityId entityId;
    };

    struct InitCacheForArchetypeCommand final : public Command
    {
        InitCacheForArchetypeCommand(Archetype& archetype)
            : Command(CommandType::InitCacheForArchetype),
              archetype(&archetype) { };
        Archetype* archetype;
    };

    MutateEntityCommand& CommandBuffer::makeMutateCommand(EntityId entity, Archetype* from, ArchetypeEntityIndex fromIndex, Archetype& to, UnsortedComponentsView addedComponents)
    {
        MutateEntityCommand& command = *allocator.create<MutateEntityCommand>(
            entity, from, fromIndex, to);

        auto componentsIndices = allocate<ArchetypeComponentIndex>(addedComponents.size());
        {
            auto it = componentsIndices;
            for (const auto componentId : addedComponents)
                *(++it) = to.GetComponentIndex(componentId);
        }

        command.componentsIndices = {componentsIndices, addedComponents.size()};
        return command;
    }

    void CommandBuffer::Destroy(EntityId entity)
    {
        ASSERT(!inProcess);

        auto& command = *allocator.create<DestroyEntityCommand>(entity);
        commands.push_back(&command);
    }

    void CommandBuffer::InitCache(Archetype& archetype)
    {
        ASSERT(!inProcess);

        auto& command = *allocator.create<InitCacheForArchetypeCommand>(archetype);
        commands.push_back(&command);
    }

    struct CommmandProcessors
    {
        static void process(MutateEntityCommand& command, World& world)
        {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)
            world.mutateEntity(command.entityId, command.from, command.fromIndex, *command.to, [&](Archetype& archetype, ArchetypeEntityIndex entityIndex) {
                ASSERT(command.componentsIndices.size() == command.componentsData.size());

                auto componentIndex = command.componentsIndices.begin();
                for (void* componentPtr : command.componentsData)
                    archetype.MoveComponentFrom(entityIndex, *(++componentIndex), componentPtr);
            });
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        static void process(DestroyEntityCommand& command, World& world)
        {
            world.destroyImpl(command.entityId);
        }

        static void process(InitCacheForArchetypeCommand& command, World& world)
        {
            world.initCache(*command.archetype);
        }
    };

    void CommandBuffer::ProcessCommands(World& world)
    {
        if (commands.empty())
            return;

        // Preventing infinite recursion: Init cache commands execute queries that lock/unlock the world,
        // which triggers command buffer flush on unlock.
        // The ASSERT(!inProcess) in command creation ensures we don't add new commands while processing existing ones.
        if (inProcess)
            return;

        inProcess = true;

#define PROCESS_COMMAND(commandType)                                                      \
    case CommandType::commandType:                                                        \
        CommmandProcessors::process(*static_cast<commandType##Command*>(command), world); \
        break;

        for (auto command : commands)
        {
            switch (command->type)
            {
                PROCESS_COMMAND(MutateEntity)
                PROCESS_COMMAND(DestroyEntity)
                PROCESS_COMMAND(InitCacheForArchetype)
            default:
                ASSERT_MSG(false, "Unknown command type");
            }
        }

        #undef PROCESS_COMMAND

        inProcess = false;
        allocator.reset();
        commands.clear();
    }
}

