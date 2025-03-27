#pragma once

#include "EASTL/algorithm.h"
#include "ecs/Archetype.hpp"
#include <immintrin.h>

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
            const auto componentIndex = archetype.GetComponentIndex<Component>();
            if constexpr (std::is_pointer_v<Arg>)
            {
                componentDataArray = componentIndex ? archetype.GetComponentsData(componentIndex) : nullptr;
            } else {
                componentDataArray = archetype.GetComponentsData(componentIndex);
                ASSERT(data);
                ASSERT(componentDataArray);
            }
        }

        void SetChunkIndex(const Archetype& archetype, size_t chunkIndex)
        {
            data = (!eastl::is_pointer_v<Arg> || componentDataArray) ? *(componentDataArray + chunkIndex) : nullptr;
            ASSERT(!eastl::is_pointer_v<Arg> || data);
        }

        void Prefetch(const Archetype& archetype, size_t chunkIndex)
        {
            if (!(std::is_pointer_v<Arg>) || data)
                _mm_prefetch(reinterpret_cast<const char*>(*(componentDataArray + chunkIndex)), _MM_HINT_T0);
        }

        Component* Get(size_t entityIndex)
        {
            return (!eastl::is_pointer_v<Arg> || data) ? reinterpret_cast<Component*>(data) + entityIndex : nullptr;
        }

    private:
        std::byte* data;
        std::byte* const * componentDataArray;
    };

    template <typename Arg>
    struct ComponentAccessor<Arg, eastl::enable_if_t<eastl::is_same_v<Ecs::World, GetComponentType<Arg>>>>
    {
        using Argument = Arg;
        using Component = GetComponentType<Arg>;

        ComponentAccessor(const Archetype&, const IterationContext& context) : world(context.world) { };
        void SetChunkIndex(const Archetype& archetype, size_t chunkIndex) {};
        World* Get(size_t) { return &world; }

    private:
        Ecs::World& world;
    };

    template <typename Arg>
    struct ComponentAccessor<Arg, eastl::enable_if_t<eastl::is_base_of_v<Ecs::Event, GetComponentType<Arg>>>>
    {
        using Argument = Arg;
        using Component = GetComponentType<Arg>;

        ComponentAccessor(const Archetype&, const IterationContext& context) : event(context.event) { };
        void SetChunkIndex(const Archetype& archetype, size_t chunkIndex) {};

        Component* Get(size_t)
        {
            static_assert(eastl::is_const_v<eastl::remove_pointer_t<Argument>>, "Event component should be read only accessed");
            return &event->As<Component>();
        }
    private:
        const Ecs::Event* event;
    };

    struct ArchetypeIterator
    {
    private:
        template <typename Arg>
        static Arg castComponentPtrToArg(GetComponentType<Arg>* ptr)
        {
            if constexpr (eastl::is_pointer_v<Arg>)
            {
                return ptr;
            }
            else
                return *ptr;
        }

        template <typename... Args, typename... ComponentDataPtrs, typename Func>
        static void invoke(Func&& func, ComponentDataPtrs __restrict... ptrs)
        {
            func(castComponentPtrToArg<Args>(ptrs)...);
        }

        template <size_t UNROLL_N, typename Func, typename... ComponentAccessors>
        static void processChunk(Func&& func, size_t entitiesCount, ComponentAccessors... components)
        {
            #if 0 // Auto unroll worlks better
                // clang-format off
                #ifdef __GNUC__
                #error "Fix unroll for ggc"
                #endif

                #pragma unroll UNROLL_N // clang-format on
            #endif
            for (size_t i = 0; i < entitiesCount; i++)
                invoke<typename ComponentAccessors::Argument...>(eastl::forward<Func>(func), components.Get(i)...);
        }

        template <typename Func, typename... ComponentAccessors>
        static void invokeForEntity(ArchetypeEntityIndex entityIndex, Func&& func, ComponentAccessors... components)
        {
            func(castComponentPtrToArg<typename ComponentAccessors::Argument>(components.Get(entityIndex.GetIndexInChunk()))...);
        }

        template <typename ArgumentList, typename Func, size_t... Index>
        static void processEntity(const Archetype& archetype, ArchetypeEntityIndex entityIndex, Func&& func, const IterationContext& context, const eastl::index_sequence<Index...>&)
        {
            // TODO optimize finding index, based on previous finded.
            // TODO componentIndexes could be cached.
             auto componentAccessors = eastl::make_tuple(ComponentAccessor<typename ArgumentList::template Get<Index>>(archetype, context)...);
            (eastl::get<Index>(componentAccessors).SetChunkIndex(archetype, entityIndex.GetChunkIndex()), ...);

            invokeForEntity(entityIndex, eastl::forward<Func>(func), eastl::get<Index>(componentAccessors)...);
        }

        template <typename ArgumentList, typename Func, size_t... Index>
        static void processArchetype(const Archetype& archetype, Func&& func, const IterationContext& context, const eastl::index_sequence<Index...>&)
        {
            // TODO optimize finding index, based on previous finded.
            // TODO componentIndexes could be cached.
            auto componentAccessors = eastl::make_tuple(ComponentAccessor<typename ArgumentList::template Get<Index>>(archetype, context)...);

            for (size_t chunkIndex = 0, chunkCount = archetype.GetChunksCount(), entityOffset = 0; chunkIndex < chunkCount; chunkIndex++, entityOffset += archetype.GetChunkCapacity())
            {
                (eastl::get<Index>(componentAccessors).SetChunkIndex(archetype, chunkIndex), ...);
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