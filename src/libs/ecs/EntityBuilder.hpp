#pragma once

// EASTL tuple have a bugs https://github.com/electronicarts/EASTL/issues/562
// Fallback to std
#include <tuple>
#include "ecs/EntityId.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/World.hpp"
#include <EASTL/sort.h>

namespace RR::Ecs
{
    template <>
    struct [[nodiscard]] EntityBuilder<void, void>
    {
    public:

        template <typename Component, typename... ARGS>
        [[nodiscard]] auto Add(ARGS&&... args)
        {
            auto argsTuple = std::make_tuple(eastl::forward<ARGS>(args)...);
            auto tupleOfTuples = std::make_tuple(eastl::move(argsTuple));
            return EntityBuilder<TypeList<Component>, decltype(tupleOfTuples)>(
                world_, entity_, eastl::move(remove_), eastl::move(tupleOfTuples));
        }

        template <typename Component>
        [[nodiscard]] auto& Remove()
        {
            remove_.push_back_unsorted(GetComponentId<Component>);
            return *this;
        }

        Entity Apply()
        {
            eastl::quick_sort(remove_.begin(), remove_.end());
            return Entity(world_, world_.commit<TypeList<>>(entity_, {remove_.begin(), remove_.end()}, std::make_tuple()));
        }

    private:
        friend Entity;
        friend World;

        explicit EntityBuilder(World& world, EntityId entity) : world_(world), entity_(entity) { };

    private:
        World& world_;
        EntityId entity_;
        ComponentsSet remove_;
    };

    template <typename ComponentList, typename ArgsTuple>
    struct [[nodiscard]] EntityBuilder
    {
    public:
        template <typename Component, typename... ARGS>
        [[nodiscard]] auto Add(ARGS&&... args)
        {
            auto argsTuple = std::make_tuple(eastl::forward<ARGS>(args)...);
            auto tupleOfTuples = std::tuple_cat(eastl::move(args_), eastl::move(std::make_tuple(argsTuple)));
            return EntityBuilder<typename ComponentList::template Append<Component>, decltype(tupleOfTuples)>(
                world_, entity_, eastl::move(remove_), eastl::move(tupleOfTuples));
        }

        template <typename Component>
        [[nodiscard]] auto& Remove()
        {
            remove_.push_back_unsorted(GetComponentId<Component>);
            return *this;
        }

        Entity Apply()
        {
            eastl::quick_sort(remove_.begin(), remove_.end());
            return Entity(world_, world_.commit<ComponentList>(entity_, {remove_.begin(), remove_.end()}, eastl::move(args_)));
        };

    private:
        template <typename C, typename T>
        friend struct EntityBuilder;

        explicit EntityBuilder(World& world, EntityId entity, ComponentsSet&& remove, ArgsTuple&& args) : world_(world), entity_(entity), remove_(eastl::move(remove)), args_(eastl::move(args)) { };

    private:
        World& world_;
        EntityId entity_;
        ComponentsSet remove_;
        ArgsTuple args_;
    };

    inline EntityBuilder<void, void> Entity::Edit() const { return EntityBuilder<void, void>(*world, id); }
}