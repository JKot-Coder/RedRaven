#pragma once

#include "ecs/EntityId.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "EASTL/tuple.h"

namespace RR::Ecs
{
    template <typename ComponentList, typename ArgsTuple>
    class [[nodiscard]] EntityBuilder;

    template <>
    class [[nodiscard]] EntityBuilder<void, void>
    {
    public:
        template <typename Component, typename... ARGS>
        auto Add(ARGS&&... args) &&
        {
            auto componentTyple = eastl::make_tuple(eastl::move(args)...);
            return EntityBuilder<TypeList<Component>, decltype(componentTyple)>(world_, eastl::move(componentTyple));
        }

        inline void Commit() &&;

    private:
        friend World;

        explicit EntityBuilder(World& world) : world_ {world} { };

    private:
        World& world_;
    };

    template <typename ComponentList, typename ArgsTuple>
    class [[nodiscard]] EntityBuilder
    {
    public:
        template <typename Component, typename... ARGS>
        auto Add(ARGS&&... args) &&
        {
            auto argsTuple = eastl::make_tuple(make_tuple(args)...);
            return EntityBuilder<typename ComponentList::template Append<Component>, decltype(eastl::tuple_cat(eastl::move(args_), argsTuple))>
                (world_, eastl::tuple_cat(eastl::move(args_), argsTuple));
        }

        inline void Commit() &&;

        static constexpr auto GetComponentsInfoList()
        {
            return getComponentsInfoListImpl(eastl::make_index_sequence<ComponentList::Count>{});
        }

    private:
        template <typename C, typename T>
        friend class EntityBuilder;

        explicit EntityBuilder(World& world, ArgsTuple&& args) : world_(world), args_(eastl::move(args)) { };

    private:
        World& world_;
        ArgsTuple args_;
    };
}