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

                // TODO frame memory allocator
/*
                (calculateComponentOffset(
                        IsTag<typename Components::template Get<Index>> ? 0 : sizeof(typename Components::template Get<Index>),
                        IsTag<typename Components::template Get<Index>> ? 1 : alignof(typename Components::template Get<Index>)),
                    ...);*/



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
                UNUSED(args);
/*
                (constructComponent<typename Components::template Get<Index>>(
                     commandPtr + offsets[Index], eastl::forward<std::tuple_element_t<Index, ArgsTuple>>(std::get<Index>(args))),
                 ...);

                command.offsets = std::move(offsets);
                (command.addedComponents.push_back_unsorted(GetComponentId<typename Components::template Get<Index>>), ...);

                for (const auto component : removeComponents)
                    command.removeComponents.push_back_unsorted(component);
*/
                commands.push_back(&command);
            }

         /*  template <typename Components, typename ArgsTuple, size_t... Index>
            void Commit(EntityId entity, SortedComponentsView removeComponents, ArgsTuple&& args, eastl::index_sequence<Index...>)
            {
                ASSERT(entity);

                // TODO frame memory allocator
                eastl::fixed_vector<uint16_t, PreallocatedComponentsCount> offsets;
                size_t offset = sizeof(MutateEntityCommand);

                auto calculateComponentOffset = [&](size_t size, size_t alignment) {
                    AlignTo(offset, alignment);
                    offsets.push_back(static_cast<uint16_t>(offset));
                    offset += size;
                };

                (calculateComponentOffset(
                        IsTag<typename Components::template Get<Index>> ? 0 : sizeof(typename Components::template Get<Index>),
                        IsTag<typename Components::template Get<Index>> ? 1 : alignof(typename Components::template Get<Index>)),
                    ...);

                std::byte* commandPtr = static_cast<std::byte*>(allocator.allocate(offset, alignof(MutateEntityCommand)));
                auto& command = *new (commandPtr) MutateEntityCommand();
                command.entityId = entity;

                (constructComponent<typename Components::template Get<Index>>(
                     commandPtr + offsets[Index], eastl::forward<std::tuple_element_t<Index, ArgsTuple>>(std::get<Index>(args))),
                 ...);

                command.offsets = std::move(offsets);
                (command.addedComponents.push_back_unsorted(GetComponentId<typename Components::template Get<Index>>), ...);

                for (const auto component : removeComponents)
                    command.removeComponents.push_back_unsorted(component);

                commands.push_back(&command);
            }*/

        private:
            Common::ChunkAllocator allocator;
            eastl::vector<Command*> commands;
    };
}
