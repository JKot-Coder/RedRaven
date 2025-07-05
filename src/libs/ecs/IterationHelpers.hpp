#pragma once

#include "EASTL/algorithm.h"
#include "ecs/Archetype.hpp"
#include <ecs/FunctionTraits.hpp>
#if defined(__i386__) || defined(__x86_64__)
#include <immintrin.h>
#endif

namespace RR::Ecs
{
    struct ArchetypeEntitySpan
    {
        ArchetypeEntitySpan(const Archetype& archetype, ArchetypeEntityIndex begin, ArchetypeEntityIndex end)
            : archetype(&archetype), begin(begin), end(end) { };
        const Archetype* archetype;
        ArchetypeEntityIndex begin;
        ArchetypeEntityIndex end;
    };

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
                ASSERT(componentDataArray);
            }
        }

        void SetChunkIndex([[maybe_unused]] const Archetype& archetype, size_t chunkIndex)
        {
            ASSERT(chunkIndex < archetype.GetChunksCount());
            if constexpr (eastl::is_pointer_v<Arg>)
            {
                data = componentDataArray ? *(componentDataArray + chunkIndex) : nullptr;
            }
            else
            {
                data = *(componentDataArray + chunkIndex);
                ASSERT(data);
            }

        }

        void Prefetch(size_t chunkIndex)
        {
            UNUSED(chunkIndex);

#if defined(__i386__) || defined(__x86_64__)
            if constexpr (std::is_pointer_v<Arg>)
            {
                if (data)
                    _mm_prefetch(reinterpret_cast<const char*>(*(componentDataArray + chunkIndex)), _MM_HINT_T0);
            }
            else
            {
                _mm_prefetch(reinterpret_cast<const char*>(*(componentDataArray + chunkIndex)), _MM_HINT_T0);
            }
#endif
        }

        Component* Get(size_t entityIndex)
        {
            if constexpr (eastl::is_pointer_v<Arg>)
            {
                return data ? reinterpret_cast<Component*>(data) + entityIndex : nullptr;
            }
            else
            {
                return reinterpret_cast<Component*>(data) + entityIndex;
            }
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
        void SetChunkIndex([[maybe_unused]] const Archetype& archetype,[[maybe_unused]] size_t chunkIndex) { ASSERT(chunkIndex < archetype.GetChunksCount()); };
        World* Get(size_t) { return &world; }

    private:
        Ecs::World& world;
    };

    template <typename Arg>
    struct ComponentAccessor<Arg, eastl::enable_if_t<eastl::is_base_of_v<Ecs::Event, GetComponentType<Arg>>>>
    {
        using Argument = Arg;
        using Component = GetComponentType<Arg>;

        ComponentAccessor(const Archetype&, const IterationContext& context) : event(context.event) {
            static_assert(eastl::is_pointer_v<Argument> || eastl::is_reference_v<Argument>, "Event component should be accessed as pointer or reference");
        };
        void SetChunkIndex([[maybe_unused]] const Archetype& archetype, [[maybe_unused]] size_t chunkIndex) { ASSERT(chunkIndex < archetype.GetChunksCount()); };

        const Component* Get(size_t)
        {
            using T = eastl::remove_pointer_t<eastl::remove_reference_t<Argument>>;
            static_assert(eastl::is_const_v<T>, "Event component should be read only accessed");

            return &event->As<Component>();
        }
    private:
        const Ecs::Event* event;
    };

    struct ArchetypeIterator
    {
    private:
        template <typename Arg, typename Ptr>
        static Arg castComponentPtrToArg(Ptr* ptr)
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
        static void processChunk(Func&& func, uint32_t beginEntityIndex, uint32_t endEntityIndex, ComponentAccessors... components)
        {
            // clang-format off
            #if 0
                // Auto unroll works better
                #ifdef __GNUC__
                #error "Fix unroll for ggc"
                #endif
                #pragma unroll UNROLL_N
            #endif // clang-format on
            for (uint32_t i = beginEntityIndex; i < endEntityIndex; i++)
                invoke<typename ComponentAccessors::Argument...>(eastl::forward<Func>(func), components.Get(i)...);
        }

        template <typename Func, typename... ComponentAccessors>
        static void invokeForEntity(uint32_t indexInChunk, Func&& func, ComponentAccessors&... components)
        {
            func(castComponentPtrToArg<typename ComponentAccessors::Argument>(components.Get(indexInChunk))...);
        }

        template <typename ArgumentList, typename Func, size_t... Index>
        static void processEntity(const Archetype& archetype, ArchetypeEntityIndex entityIndex, const IterationContext& context, Func&& func, const eastl::index_sequence<Index...>&)
        {
            // TODO optimize finding index, based on previous finded.
            // TODO componentIndexes could be cached.
             auto componentAccessors = eastl::make_tuple(ComponentAccessor<typename ArgumentList::template Get<Index>>(archetype, context)...);
            (eastl::get<Index>(componentAccessors).SetChunkIndex(archetype, entityIndex.GetChunkIndex()), ...);

            invokeForEntity(entityIndex.GetIndexInChunk(), eastl::forward<Func>(func), eastl::get<Index>(componentAccessors)...);
        }

        template <typename ArgumentList, typename Func, size_t... Index>
        static void processEntities(const ArchetypeEntitySpan& span, const IterationContext& context, Func&& func, const eastl::index_sequence<Index...>&)
        {
            const Archetype& archetype = *span.archetype;
            if (archetype.GetEntitiesCount() == 0)
                return;

            // TODO optimize finding index, based on previous finded.
            // TODO componentIndexes could be cached.
            auto componentAccessors = eastl::make_tuple(ComponentAccessor<typename ArgumentList::template Get<Index>>(archetype, context)...);

            uint32_t beginChunkIndex = static_cast<uint32_t>(span.begin.GetChunkIndex());
            uint32_t endChunkIndex = static_cast<uint32_t>(span.end.GetChunkIndex());

            uint32_t beginEntityIndex = span.begin.GetIndexInChunk();
            uint32_t endEntityIndex = static_cast<uint32_t>(archetype.GetChunkCapacity());

            for (uint32_t chunkIndex = beginChunkIndex; chunkIndex <= endChunkIndex; ++chunkIndex)
            {
                if (chunkIndex == endChunkIndex)
                    endEntityIndex = span.end.GetIndexInChunk();

                (eastl::get<Index>(componentAccessors).SetChunkIndex(archetype, chunkIndex), ...);
                processChunk<4>(eastl::forward<Func>(func), beginEntityIndex, endEntityIndex, eastl::get<Index>(componentAccessors)...);

                if (chunkIndex == beginChunkIndex)
                    beginEntityIndex = 0;
            }
        }

    public:
        template <typename Callable>
        static void ForEach(ArchetypeEntitySpan span, const IterationContext& context, Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            processEntities<ArgList>(span, context, eastl::forward<Callable>(callable), eastl::make_index_sequence<ArgList::Count>());
        }

        template <typename Callable>
        static void ForEntity(const Archetype& archetype, ArchetypeEntityIndex entityId, const IterationContext& context, Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            processEntity<ArgList>(archetype, entityId, context, eastl::forward<Callable>(callable), eastl::make_index_sequence<ArgList::Count>());
        }
    };
}