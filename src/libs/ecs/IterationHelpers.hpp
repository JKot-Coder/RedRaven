#pragma once

#include "EASTL/algorithm.h"
#include "ecs/Archetype.hpp"

namespace RR::Ecs
{
    template <typename Arg>
    struct ComponentAccessor
    {
        using Argument = Arg;
        using Component = GetComponentType<Arg>;

        ComponentAccessor(const Archetype& archetype)
        {
            data = archetype.GetComponentData(GetComponentId<Component>);
            ASSERT(data);
        }

        Arg& Get(size_t chunkIndex, size_t index)
        {
            Component* ptr = reinterpret_cast<Component*>(data->GetData(chunkIndex, index));

            if constexpr (std::is_pointer_v<Arg>)
            {
                return ptr;
            }
            else
                return *ptr;
        }

        const Archetype::ComponentData* data;
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

        template <typename ArgumentList, typename Func, size_t... Index>
        static void iterateArchetypeImpl(Func&& func, const Archetype& archetype, const eastl::index_sequence<Index...>&)
        {
            auto componentAccessors = eastl::make_tuple(ComponentAccessor<typename ArgumentList::template Get<Index>>(archetype)...);

            for (size_t chunkIdx = 0, chunkCount = archetype.GetChunkCount(), entityOffset = 0; chunkIdx < chunkCount; chunkIdx++, entityOffset += archetype.GetChunkSize())
            {
                size_t entitiesCount = eastl::min(archetype.GetEntityCount() - entityOffset, archetype.GetChunkSize());
                processChunk<4>(eastl::forward<Func>(func), chunkIdx, entitiesCount, eastl::get<Index>(componentAccessors)...);
            }
        }

    public:
        template <typename Callable>
        static void ForEach(Callable&& callable, const Archetype& archetype)
        {
            using ArgList = GetArgumentList<Callable>;
            iterateArchetypeImpl<ArgList>(eastl::forward<Callable>(callable), archetype, eastl::make_index_sequence<ArgList::Count>());
        }
    };
}