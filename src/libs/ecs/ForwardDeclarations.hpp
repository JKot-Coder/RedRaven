
#pragma once

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

    template <typename T>
    struct EventBuilder;
    template <typename... Components>
    struct SystemBuilder;
    template <typename T>
    class EntityBuilder;

    template <typename C, typename Args>
    struct ComponentArg
    {
        using Component = C;
        ComponentArg(Args&& args_) : args(eastl::move(args_)) { }
        static constexpr size_t num_args = eastl::tuple_size<Args>::value;
        Args args;
    };
}