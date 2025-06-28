#pragma once

#include "ecs/ComponentTraits.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/ForwardDeclarations.hpp"

#include "common/ChunkAllocator.hpp"

#include "EASTL/span.h"

namespace RR::Ecs
{
    enum class CommandType : uint8_t;

    struct Command
    {
        CommandType type;
        Command(CommandType type) : type(type) {};
    };

    enum class CommandType : uint8_t
    {
        MutateEntity,
        DestroyEntity,
        InitCacheForArchetype
    };

    struct MutateEntityCommand final : public Command
    {
        MutateEntityCommand(EntityId entityId, Archetype* from, Archetype& to)
            : Command(CommandType::MutateEntity),
              entityId(entityId),
#ifdef ENABLE_ASSERTS
              from(from),
#endif
              to(&to) {
                  UNUSED(from);
              };
        EntityId entityId;
#ifdef ENABLE_ASSERTS
        Archetype* from;
#endif
        Archetype* to;
        eastl::span<ArchetypeComponentIndex> componentsIndices;
        eastl::span<void*> componentsData;
    };

    struct CommandBuffer final
    {
        private:
            static constexpr size_t InitialCommandQueueSize = 1024*1024;

        private:
            MutateEntityCommand& makeMutateCommand(EntityId entity, Archetype* from, Archetype& to, UnsortedComponentsView addedComponents);

            template <typename T>
            T* allocate(size_t count)
            {
                return static_cast<T*>(allocator.allocate(count * sizeof(T), alignof(T)));
            }

            template <typename Component, typename ArgsTuple>
            void* constructComponent(ArgsTuple&& args)
            {
                void* result = nullptr;

                if constexpr (IsTag<Component>)
                    return result;

                std::apply(
                    [&result, this](auto&&... unpackedArgs) {
                        void* memory = allocator.allocate(sizeof(Component), alignof(Component));
                        result = new (memory) Component {eastl::forward<decltype(unpackedArgs)>(unpackedArgs)...};
                    },
                    eastl::forward<ArgsTuple>(args));

                return result;
            }

        public:
            CommandBuffer() : allocator(InitialCommandQueueSize) { }

            void ProcessCommands(World& world);

            template <typename Components, typename ArgsTuple, size_t... Index>
            void Mutate(EntityId entity, Archetype* from, Archetype& to, ArgsTuple&& args, eastl::index_sequence<Index...>)
            {
                ASSERT(entity);
                ASSERT(!inProcess);

                const eastl::array<ComponentId, Components::Count> componentIds = {GetComponentId<typename Components::template Get<Index>>...};
                auto& command = makeMutateCommand(entity, from, to, UnsortedComponentsView(componentIds));

                void** componentsPtrs = allocate<void*>(Components::Count);
                (void( *(componentsPtrs + Index) = constructComponent<typename Components::template Get<Index>>(
                    eastl::forward<std::tuple_element_t<Index, ArgsTuple>>(std::get<Index>(args)))
                ), ...);

                command.componentsData = {componentsPtrs, Components::Count};
                commands.push_back(&command);
            }

            void Destroy(EntityId entity);
            void InitCache(Archetype& archetype);

        private:
            bool inProcess = false;
            Common::ChunkAllocator allocator;
            eastl::vector<Command*> commands;
    };
}
