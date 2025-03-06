#pragma once

#include "EASTL/algorithm.h"
#include "ecs/Archetype.hpp"

namespace RR::Ecs
{
    struct IterationContext
    {
        IterationContext(Ecs::World& world, const Archetype& archetype) : world(world), archetype(archetype) { };
        Ecs::World& world;
        const Archetype& archetype;
    };

    template <typename Arg, typename Enable = void>
    struct ComponentAccessor
    {
        using Argument = Arg;
        using Component = GetComponentType<Arg>;

        ComponentAccessor(const IterationContext& context)
        {
            data = context.archetype.GetComponentData(GetComponentId<Component>);
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

        ComponentAccessor(const IterationContext& context) : world(context.world) { };
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
        static void processEntity(ArchetypeEntityIndex entityIndex, Func&& func, const IterationContext& context, const eastl::index_sequence<Index...>&)
        {
            auto componentAccessors = eastl::make_tuple(ComponentAccessor<typename ArgumentList::template Get<Index>>(context)...);

            invokeForEntity(entityIndex, eastl::forward<Func>(func), eastl::get<Index>(componentAccessors)...);
        }

        template <typename ArgumentList, typename Func, size_t... Index>
        static void processArchetype(Func&& func, const IterationContext& context, const eastl::index_sequence<Index...>&)
        {
            auto componentAccessors = eastl::make_tuple(ComponentAccessor<typename ArgumentList::template Get<Index>>(context)...);

            for (size_t chunkIndex = 0, chunkCount = context.archetype.GetChunkCount(), entityOffset = 0; chunkIndex < chunkCount; chunkIndex++, entityOffset += context.archetype.GetChunkSize())
            {
                size_t entitiesCount = eastl::min(context.archetype.GetEntityCount() - entityOffset, context.archetype.GetChunkSize());
                processChunk<4>(eastl::forward<Func>(func), chunkIndex, entitiesCount, eastl::get<Index>(componentAccessors)...);
            }
        }

    public:
        template <typename Callable>
        static void ForEach(Callable&& callable, const IterationContext& context)
        {
            using ArgList = GetArgumentList<Callable>;
            processArchetype<ArgList>(eastl::forward<Callable>(callable), context, eastl::make_index_sequence<ArgList::Count>());
        }

        template <typename Callable>
        static void ForEntity(ArchetypeEntityIndex entityId, Callable&& callable, const IterationContext& context)
        {
            using ArgList = GetArgumentList<Callable>;
            processEntity<ArgList>(entityId, eastl::forward<Callable>(callable), context, eastl::make_index_sequence<ArgList::Count>());
        }
    };
}