#include "CommandBuffer.hpp"

#include "ecs/World.hpp"
#include "ecs/ComponentTraits.hpp"

namespace RR::Ecs
{
    enum class CommandType : uint8_t
    {
        MutateEntity,
        DestroyEntity,
        AddComponent,
        RemoveComponent,
    };

    struct MutateEntityCommand : Command
    {
        EntityId entityId;
        ComponentsSet removeComponents;
        ComponentsSet addedComponents;

        struct ComponentDataHeader
        {
            uint16_t alignment;
            uint16_t componentArchetypeIndex;
        };
    };

    struct CommmandProcessors
    {
        static void process(MutateEntityCommand& command, World& world)
        {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)
            world.mutateEntity(command.entityId, command.removeComponents, command.addedComponents, [&](Archetype& archetype, ArchetypeEntityIndex index) {
                auto* data_ptr = reinterpret_cast<std::byte*>(&command);
                data_ptr += sizeof(MutateEntityCommand);

                for (auto _ : command.addedComponents)
                {
                    using ComponentDataHeader = MutateEntityCommand::ComponentDataHeader;
                    const auto& componentData = *reinterpret_cast<ComponentDataHeader*>(data_ptr);
                    const ArchetypeComponentIndex componentIndex = ArchetypeComponentIndex(componentData.componentArchetypeIndex);

                    const auto& componentInfo = archetype.GetComponentInfo(componentIndex);
                    data_ptr += sizeof(ComponentDataHeader);
                    data_ptr += componentData.alignment;

                    archetype.MoveComponentFrom(index, componentIndex, data_ptr);
                    data_ptr += componentInfo.size;
                }
            });
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)
        }
    };

    void CommandBuffer::ProcessCommands(World& world)
    {
        #define PROCESS_COMMAND(commandType) \
            case CommandType::commandType: \
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
    }
}

