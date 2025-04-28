#pragma once

#include "ecs/EntityId.hpp"
#include "ecs/ComponentTraits.hpp"
#include "common/ChunkAllocator.hpp"
#include "EASTL/span.h"

namespace RR::Ecs
{
    struct World;
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
        MutateEntityCommand(EntityId entityId, UnsortedComponentsView addedComponents, SortedComponentsView removedComponents)
            : Command(CommandType::MutateEntity),
              entityId(entityId),
              addedComponents(addedComponents),
              removedComponents(removedComponents) { };
        EntityId entityId;
        UnsortedComponentsView addedComponents;
        SortedComponentsView removedComponents;
        eastl::span<void*> components;

       // eastl::fixed_vector<uint16_t, PreallocatedComponentsCount> offsets;
    };

    struct CommandBuffer final
    {
        private:
            static constexpr size_t InitialCommandQueueSize = 1024*1024;

        private:
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

        public:
            CommandBuffer() : allocator(InitialCommandQueueSize) { }

            void ProcessCommands(World& world);

            template<typename Iterator>
            auto* AllocateElements(Iterator begin, Iterator end) {
                using ElemType = eastl::remove_cv_t<typename eastl::iterator_traits<Iterator>::value_type>;

                size_t count = eastl::distance(begin, end);
                size_t totalSize = count * sizeof(ElemType);

                ElemType* firstElement = static_cast<ElemType*>(allocator.allocate(totalSize, alignof(ElemType)));
                ElemType* currentElement = firstElement;
                for (auto it = begin; it != end; ++it)
                    new (currentElement++) ElemType(*it);

                return firstElement;
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

            template <typename Components, typename ArgsTuple, size_t... Index>
            void Commit(EntityId entity, SortedComponentsView removeComponents, UnsortedComponentsView addedComponents, ArgsTuple&& args, eastl::index_sequence<Index...>)
            {
                ASSERT(entity);

                auto& command = *allocator.allocateTyped<MutateEntityCommand>(
                    entity,
                    UnsortedComponentsView {AllocateElements(addedComponents.begin(), addedComponents.end()),
                                            eastl::distance(addedComponents.begin(), addedComponents.end())},
                    SortedComponentsView {AllocateElements(removeComponents.begin(), removeComponents.end()),
                                          eastl::distance(removeComponents.begin(), removeComponents.end())}
                );

                void** componentsPtrs = static_cast<void**>(allocator.allocate(sizeof(void*) * Components::Count, alignof(void*)));

                (void( *(componentsPtrs + Index) = constructComponent<typename Components::template Get<Index>>(
                    eastl::forward<std::tuple_element_t<Index, ArgsTuple>>(std::get<Index>(args)))
                ), ...);

                command.components = {componentsPtrs, Components::Count};
                commands.push_back(&command);
            }

        private:
            Common::ChunkAllocator allocator;
            eastl::vector<Command*> commands;
    };
}
