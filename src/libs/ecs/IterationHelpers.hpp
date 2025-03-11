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

        ComponentAccessor(const Archetype& archetype, const IterationContext&)
        {
            data = archetype.GetComponentData(GetComponentId<Component>);
            ASSERT(data);
        }

        Arg Get(ArchetypeEntityIndex entityIndex)
        {
            return dereference(reinterpret_cast<Component*>(data->GetData(entityIndex)));
        }

        Arg Get(size_t chunkIndex, size_t index)
        {
            return dereference(reinterpret_cast<Component*>(data->GetData(chunkIndex, index)));
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

        const Archetype::ComponentData* data;
    };

    template <typename Arg>
    struct ComponentAccessor<Arg, eastl::enable_if_t<eastl::is_same_v<Ecs::World, GetComponentType<Arg>>>>
    {
        using Argument = Arg;
        using Component = GetComponentType<Arg>;

        ComponentAccessor(const Archetype&, const IterationContext& context) : world(context.world) { };
        Arg Get(ArchetypeEntityIndex) { return dereference(); }
        Arg Get(size_t, size_t) { return dereference(); }

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

    struct ArchetypeIterator
    {
    private:
        template <size_t UNROLL_N, typename Func, typename... ComponentAccessors>
        static void processChunk(Func&& func, size_t chunkIndex, size_t entitiesCount, ComponentAccessors... components)
        {
            // clang-format off
            #ifdef __GNUC__
            #error "Fix unroll for ggc"
            #endif

            #pragma unroll UNROLL_N // clang-format on
            for (size_t i = 0; i < entitiesCount; i++)
            {
                func(eastl::forward<typename ComponentAccessors::Argument>(components.Get(chunkIndex, i))...);
            }
        }

        template <typename Func, typename... ComponentAccessors>
        static void invokeForEntity(ArchetypeEntityIndex entityIndex, Func&& func, ComponentAccessors... components)
        {
            func(eastl::forward<typename ComponentAccessors::Argument>(components.Get(entityIndex))...);
        }

        template <typename ArgumentList, typename Func, size_t... Index>
        static void processEntity(const Archetype& archetype, ArchetypeEntityIndex entityIndex, Func&& func, const IterationContext& context, const eastl::index_sequence<Index...>&)
        {
            auto componentAccessors = eastl::make_tuple(ComponentAccessor<typename ArgumentList::template Get<Index>>(archetype, context)...);

            invokeForEntity(entityIndex, eastl::forward<Func>(func), eastl::get<Index>(componentAccessors)...);
        }

        template <typename ArgumentList, typename Func, size_t... Index>
        static void processArchetype(const Archetype& archetype, Func&& func, const IterationContext& context, const eastl::index_sequence<Index...>&)
        {
            auto componentAccessors = eastl::make_tuple(ComponentAccessor<typename ArgumentList::template Get<Index>>(archetype, context)...);

            for (size_t chunkIndex = 0, chunkCount = archetype.GetChunkCount(), entityOffset = 0; chunkIndex < chunkCount; chunkIndex++, entityOffset += archetype.GetChunkSize())
            {
                size_t entitiesCount = eastl::min(archetype.GetEntityCount() - entityOffset, archetype.GetChunkSize());
                processChunk<4>(eastl::forward<Func>(func), chunkIndex, entitiesCount, eastl::get<Index>(componentAccessors)...);
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