#pragma once

// EASTL tuple have a bugs https://github.com/electronicarts/EASTL/issues/562
// Fallback to std 
#include <tuple>
#include "ecs/EntityId.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/World.hpp"

namespace RR::Ecs
{
    template <typename ComponentList, typename ArgsTuple>
    struct [[nodiscard]] EntityBuilder;

    template <>
    struct [[nodiscard]] EntityBuilder<void, void>;
    struct Entity
    {
    public:
        EntityId GetId() const { return entity_; }
        void Destruct() { world_.Destruct(entity_); }
        bool IsAlive() const { return world_.IsAlive(entity_); }

        template <typename... Components>
        bool Has()
        {
            eastl::array<ComponentId, sizeof...(Components)> components = {GetComponentId<Components>...};
            eastl::quick_sort(components.begin(), components.end());
            return world_.Has(entity_, components.begin(), components.end());
        }

        EntityBuilder<void, void> Edit();

    private:
        friend World;
        template <typename C, typename A>
        friend struct EntityBuilder;

        Entity(World& world, EntityId entity) : world_(world), entity_(entity) { }

    private:
        World& world_;
        EntityId entity_;
    };

    template <>
    struct [[nodiscard]] EntityBuilder<void, void>
    {
    public:
        template <typename Component, typename... ARGS>
        auto Add(ARGS&&... args) &&
        {
            auto argsTuple = std::make_tuple(eastl::forward<ARGS>(args)...);
            auto tupleOfTuples = std::make_tuple(eastl::move(argsTuple));
            return EntityBuilder<TypeList<Component>, decltype(tupleOfTuples)>(
                world_, entity_, eastl::move(remove_), eastl::move(tupleOfTuples));
        }
        template <typename Component>
        auto Remove() &&
        {
            remove_.push_back_unsorted(GetComponentId<Component>);
            return *this;
        }

        Entity Apply() &&
        {
            eastl::quick_sort(remove_.begin(), remove_.end());
            return Entity(world_, world_.commit<TypeList<>>(entity_, {remove_.begin(), remove_.end()}, std::make_tuple()));
        }

    private:
        friend Entity;

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
        auto Add(ARGS&&... args) &&
        {
            auto argsTuple = std::make_tuple(eastl::forward<ARGS>(args)...);
            auto tupleOfTuples = std::tuple_cat(eastl::move(args_), eastl::move(std::make_tuple(argsTuple)));
            return EntityBuilder<typename ComponentList::template Append<Component>, decltype(tupleOfTuples)>(
                world_, entity_, eastl::move(remove_), eastl::move(tupleOfTuples));
        }

        Entity Apply() &&
        {
            eastl::quick_sort(remove_.begin(), remove_.end());
            return Entity(world_, world_.commit<ComponentList>(entity_, {remove_.begin(), remove_.end()}, eastl::move(args_)));
        };

        template <typename Component>
        auto Remove() &&
        {
            remove_.push_back_unsorted(GetComponentId<Component>);
            return *this;
        }

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

    inline EntityBuilder<void, void> Entity::Edit() { return EntityBuilder<void, void>(world_, entity_); }
}