#pragma once

#include "EASTL/algorithm.h"
#include "ecs/Archetype.hpp"

namespace RR::Ecs
{
    struct IterationContext
    {
        IterationContext(World& world, const Event* event) : world(world), event(event) { };
        World& world;
        const Event* event;
    };

    template <typename Arg, typename Enable = void>
    struct ComponentAccessor
    {
        using Argument = Arg;
        using Component = GetComponentType<Arg>;

        ComponentAccessor(const Archetype& archetype, ArchetypeComponentIndex componentIndex, size_t chunkIndex, const IterationContext&)
        {
            if constexpr (std::is_pointer_v<Arg>)
            {
                data = componentIndex ? archetype.GetData(componentIndex, chunkIndex) : nullptr;
                ASSERT(!componentIndex || data);
            }
            else
            {
                ASSERT(componentIndex);
                data = archetype.GetData(componentIndex, chunkIndex);
                ASSERT(data);
            }
        }

        static ArchetypeComponentIndex GetComponentIndex(const Archetype& archetype)
        {
            return archetype.GetComponentIndex(GetComponentId<Component>);
        }

        Arg Get(size_t entityIndex)
        {
            return dereference(reinterpret_cast<Component*>(data + sizeof(Component) * entityIndex));
        }

    private:
        static Arg dereference(Component* ptr)
        {
            if constexpr (std::is_pointer_v<Arg>)
            {
                return ptr;
            }
            else
                return *ptr;
        }

        std::byte* data;
    };

    template <typename Arg>
    struct ComponentAccessor<Arg, eastl::enable_if_t<eastl::is_same_v<Ecs::World, GetComponentType<Arg>>>>
    {
        using Argument = Arg;
        using Component = GetComponentType<Arg>;

        ComponentAccessor(const Archetype&, ArchetypeComponentIndex, size_t, const IterationContext& context) : world(context.world) { };
        static ArchetypeComponentIndex GetComponentIndex(const Archetype&) { return {}; }
        Arg Get(size_t) { return dereference(); }

    private:
        Arg dereference()
        {
            if constexpr (std::is_pointer_v<Arg>)
            {
                return &world;
            }
            else
                return world;
        }

        Ecs::World& world;
    };

    template <typename Arg>
    struct ComponentAccessor<Arg, eastl::enable_if_t<eastl::is_base_of_v<Ecs::Event, GetComponentType<Arg>>>>
    {
        using Argument = Arg;
        using Component = GetComponentType<Arg>;

        ComponentAccessor(const Archetype&, ArchetypeComponentIndex, size_t, const IterationContext& context) : event(context.event) { };
        static ArchetypeComponentIndex GetComponentIndex(const Archetype&) { return {}; }
        Arg Get(size_t) { return dereference(); }

    private:
        Arg dereference()
        {
            if constexpr (eastl::is_pointer_v<Argument>)
            {
                static_assert(eastl::is_const_v<eastl::remove_pointer_t<Argument>>, "Event component should be read only accessed");
                return event->As<Component>();
            }
            else
            {
                static_assert(eastl::is_const_v<eastl::remove_reference_t<Argument>>, "Event component should be read only accessed");
                return *event->As<Component>();
            }
        }

        const Ecs::Event* event;
    };

    struct ArchetypeIterator
    {
    private:
        template <size_t UNROLL_N, typename Func, typename... ComponentAccessors>
        static void processChunk(Func&& func, size_t entitiesCount, ComponentAccessors... components)
        {
            // clang-format off
            #ifdef __GNUC__
            #error "Fix unroll for ggc"
            #endif

            #pragma unroll UNROLL_N // clang-format on
            for (size_t i = 0; i < entitiesCount; i++)
            {
                func(eastl::forward<typename ComponentAccessors::Argument>(components.Get(i))...);
            }
        }

        template <typename Func, typename... ComponentAccessors>
        static void invokeForEntity(ArchetypeEntityIndex entityIndex, Func&& func, ComponentAccessors... components)
        {
            func(eastl::forward<typename ComponentAccessors::Argument>(components.Get(entityIndex.GetIndexInChunk()))...);
        }

        template <typename ArgumentList, typename Func, size_t... Index>
        static void processEntity(const Archetype& archetype, ArchetypeEntityIndex entityIndex, Func&& func, const IterationContext& context, const eastl::index_sequence<Index...>&)
        {
            // TODO optimize finding index, based on previous finded.
            std::array<ArchetypeComponentIndex, sizeof...(Index)> componentIndexes = {ComponentAccessor<typename ArgumentList::template Get<Index>>::GetComponentIndex(archetype)...};
            auto componentAccessors = eastl::make_tuple(ComponentAccessor<typename ArgumentList::template Get<Index>>(archetype, componentIndexes[Index], entityIndex.GetChunkIndex(), context)...);

            invokeForEntity(entityIndex, eastl::forward<Func>(func), eastl::get<Index>(componentAccessors)...);
        }

        template <typename ArgumentList, typename Func, size_t... Index>
        static void processArchetype(const Archetype& archetype, Func&& func, const IterationContext& context, const eastl::index_sequence<Index...>&)
        {
            // TODO optimize finding index, based on previous finded.
            std::array<ArchetypeComponentIndex, sizeof...(Index)> componentIndexes = {ComponentAccessor<typename ArgumentList::template Get<Index>>::GetComponentIndex(archetype)...};

            for (size_t chunkIndex = 0, chunkCount = archetype.GetChunksCount(), entityOffset = 0; chunkIndex < chunkCount; chunkIndex++, entityOffset += archetype.GetChunkCapacity())
            {
                auto componentAccessors = eastl::make_tuple(ComponentAccessor<typename ArgumentList::template Get<Index>>(archetype, componentIndexes[Index], chunkIndex, context)...);

                size_t entitiesCount = eastl::min(archetype.GetEntitiesCount() - entityOffset, archetype.GetChunkCapacity());
                processChunk<4>(eastl::forward<Func>(func), entitiesCount, eastl::get<Index>(componentAccessors)...);
            }
        }

    public:
        template <typename Callable>
        static void ForEach(const Archetype& archetype, Callable&& callable, const IterationContext& context)
        {
            using ArgList = GetArgumentList<Callable>;
            processArchetype<ArgList>(archetype, eastl::forward<Callable>(callable), context, eastl::make_index_sequence<ArgList::Count>());
        }

        template <typename Callable>
        static void ForEntity(const Archetype& archetype, ArchetypeEntityIndex entityId, Callable&& callable, const IterationContext& context)
        {
            using ArgList = GetArgumentList<Callable>;
            processEntity<ArgList>(archetype, entityId, eastl::forward<Callable>(callable), context, eastl::make_index_sequence<ArgList::Count>());
        }
    };
}