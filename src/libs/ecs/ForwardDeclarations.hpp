
#pragma once

#include "EASTL/span.h" 

namespace RR::Ecs
{
    template<int FixedSize>
    struct HashString;
    using HashType = uint64_t;
    struct EntityId;
    struct Entity;
    struct Event;
    struct World;

    struct SystemDescription;

    struct EntityStorage;
    template <typename T>
    struct EventBuilder;
    struct SystemBuilder;
    template <typename C, typename T>
    struct EntityBuilder;
    struct QueryBuilder;
    struct ArchetypeEntityIndex;
    class Archetype;

    using MatchedArchetypeSpan = eastl::span<const Archetype*>; //TODO maybe move somewhre

    /**
     * @brief List of types with compile-time access
     * @tparam Types Variadic template parameter pack of types
     */
    template <typename... Types>
    struct TypeList
    {
        static constexpr size_t Count = sizeof...(Types);

        template <size_t Index>
        using Get = eastl::tuple_element_t<Index, eastl::tuple<Types...>>;

        template <typename Type>
        using Append = TypeList<Types..., Type>;
    };
}