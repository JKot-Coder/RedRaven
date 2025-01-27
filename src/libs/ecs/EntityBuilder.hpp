#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/EntityId.hpp"

namespace RR::Ecs
{
    template <typename TupleType>
    class [[nodiscard]] EntityBuilder;

    template <>
    class [[nodiscard]] EntityBuilder<void>
    {
    public:
        template <typename Component, typename... ARGS>
        auto Add(ARGS&&... args) &&;

        inline void Commit() &&;

    private:
        friend World;

        explicit EntityBuilder(World& world) : world_ {world} { };

    private:
        World& world_;
    };

    template <typename TupleType>
    class [[nodiscard]] EntityBuilder
    {
    public:
        template <typename Component, typename... ARGS>
        auto Add(ARGS&&... args) && {
            using ComponentArgType = ComponentArg<Component, eastl::tuple<ARGS...>>;
            ComponentArgType arg {eastl::tuple<ARGS...>(eastl::forward<ARGS>(args)...)};
            using NewTupleType = decltype(eastl::tuple_cat(eastl::declval<TupleType>(), eastl::make_tuple(eastl::move(arg))));
            return EntityBuilder<NewTupleType>(world_, eastl::tuple_cat(eastl::move(args_), eastl::make_tuple(eastl::move(arg))));
        }

        inline void Commit() &&;
    private:
        template <typename U>
        friend class EntityBuilder;

        explicit EntityBuilder(World& world, TupleType&& args) : world_(world), args_(eastl::move(args)) { };

    private:
        World& world_;
        TupleType args_;
    };

    template <typename Component, typename... ARGS>
    auto EntityBuilder<void>::Add(ARGS&&... args) &&
    {
        using ComponentArgType = ComponentArg<Component, eastl::tuple<ARGS...>>;
        ComponentArgType arg {eastl::tuple<ARGS...>(eastl::forward<ARGS>(args)...)};
        using NewTupleType = eastl::tuple<ComponentArgType>;
        return EntityBuilder<NewTupleType>(world_, eastl::make_tuple(eastl::move(arg)));
    }

}