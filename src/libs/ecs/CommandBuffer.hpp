#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/ArchetypeEntityIndex.hpp"
#include "ecs/ComponentTraits.hpp"
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
        DestroyEntity
    };

    struct MutateEntityCommand final : public Command
    {
        MutateEntityCommand(EntityId entityId, Archetype* from, ArchetypeEntityIndex fromIndex, Archetype& to)
            : Command(CommandType::MutateEntity),
              entityId(entityId),
              fromIndex(fromIndex),
              from(from),
              to(&to){ };
        EntityId entityId;
        ArchetypeEntityIndex fromIndex;
        Archetype *from, *to;
        eastl::span<ArchetypeComponentIndex> componentsIndices;
        eastl::span<void*> componentsData;
    };

    struct CommandBuffer final
    {
        private:
            static constexpr size_t InitialCommandQueueSize = 1024*1024;

        private:
            MutateEntityCommand& makeCommitCommand(EntityId entity, Archetype* from, ArchetypeEntityIndex fromIndex, Archetype& to, UnsortedComponentsView addedComponents);

            template <typename Component, typename ArgsTuple>
            void constructComponent(void* dst, ArgsTuple&& args)
            {
                if constexpr (IsTag<Component>)
                    return;

                std::apply(
                    [dst](auto&&... unpackedArgs) {
                        new (dst) Component {std::forward<decltype(unpackedArgs)>(unpackedArgs)...};
                    },
                    eastl::forward<ArgsTuple>(args));
            }

            template <typename T>
            T* allocate(size_t count)
            {
                return static_cast<T*>(allocator.allocate(count * sizeof(T), alignof(T)));
            }

        public:
            CommandBuffer() : allocator(InitialCommandQueueSize) { }

            void ProcessCommands(World& world);

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

            template <typename Components, typename ArgsTuple, size_t... Index>
            void Commit(EntityId entity, Archetype* from, ArchetypeEntityIndex fromIndex, Archetype& to, UnsortedComponentsView addedComponents, ArgsTuple&& args, eastl::index_sequence<Index...>)
            {
                ASSERT(entity);

                auto& command = makeCommitCommand(entity, from, fromIndex, to, addedComponents);

                void** componentsPtrs = allocate<void*>(Components::Count);
                (void( *(componentsPtrs + Index) = constructComponent<typename Components::template Get<Index>>(
                    eastl::forward<std::tuple_element_t<Index, ArgsTuple>>(std::get<Index>(args)))
                ), ...);

                command.componentsData = {componentsPtrs, Components::Count};
                commands.push_back(&command);
            }

        private:
            Common::ChunkAllocator allocator;
            eastl::vector<Command*> commands;
    };
}
