#pragma once

#include "EASTL/tuple.h"
#include "EASTL/span.h"

#include <cstddef>
#include <cstdint>

#define ECS_TRACKABLE static constexpr bool Trackable = true
#define ECS_SINGLETON static constexpr bool Singleton = true

#ifdef ECS_USE_EXCEPTIONS
#include <stdexcept>
#define ECS_VERIFY(condition, ...) if (!(condition)) throw std::runtime_error(fmt::format(__VA_ARGS__))
#define ECS_ENSURE(condition, ...) if (!(condition)) throw std::runtime_error(fmt::format(__VA_ARGS__))
#define ECS_ENABLE_CHEKS true
#else
#define ECS_VERIFY(condition, ...) ASSERT_MSG(condition, __VA_ARGS__)
#define ECS_ENSURE(condition, ...) if (!(condition)) { Log::Format::Error(__VA_ARGS__); ASSERT_MSG(condition, __VA_ARGS__); }
#define ECS_ENABLE_CHEKS ENABLE_ASSERTS
#endif

namespace RR::Ecs
{
    template<int FixedSize>
    struct HashString;
    using HashType = uint32_t;
    struct EntityId;
    struct Entity;
    struct Event;
    struct World;

    struct SystemDescription;

    struct EntityStorage;
    struct SystemBuilder;
    template <typename C, typename T>
    struct EntityBuilder;
    struct QueryBuilder;
    struct ArchetypeEntityIndex;
    struct Archetype;

    template <typename Tag, typename IndexType = uint32_t>
    struct Index;

    namespace Meta
    {
        using ComponentId = Index<struct ComponentIdTag, HashType>;
    }
    using ArchetypeId = Index<struct ArchetypeIdTag, HashType>;
    using ArchetypeComponentIndex = Index<struct ArchetypeComponentIndexTag, uint8_t>;
    using EventId = Index<struct EventIdTag, HashType>;
    using SystemId = Index<struct SystemIdTag, uint32_t>;
    using MatchedArchetypeSpan = eastl::span<const Archetype*>;
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