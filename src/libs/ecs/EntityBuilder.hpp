#pragma once

#include "EASTL/tuple.h"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/World.hpp"

namespace RR::Ecs
{
    template <typename ComponentList, typename ArgsTuple>
    struct [[nodiscard]] EntityBuilder;

    template <>
    struct [[nodiscard]] EntityBuilder<void, void>
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
    struct [[nodiscard]] EntityBuilder
    {
    public:
        template <typename Component, typename... ARGS>
        auto Add(ARGS&&... args) &&
        {
            auto argsTuple = eastl::make_tuple(args...);
            return EntityBuilder<typename ComponentList::template Append<Component>, decltype(eastl::tuple_cat(eastl::move(args_), argsTuple))>(world_, eastl::tuple_cat(eastl::move(args_), argsTuple));
        }

        inline void Commit() &&;

    private:
        template <typename C, typename T>
        friend struct EntityBuilder;

        explicit EntityBuilder(World& world, ArgsTuple&& args) : world_(world), args_(eastl::move(args)) { };

    private:
        World& world_;
        ArgsTuple args_;
    };

    //  void EntityBuilder<void, void>::Commit() && { world_.createEntity(); };
    template <typename ComponentList, typename ArgsTuple>
    void EntityBuilder<ComponentList, ArgsTuple>::Commit() &&
    {
       // world_.createEntity<ComponentList>(eastl::move(args_));
       world_.commit<ComponentList>(eastl::move(args_));
    };

    /*    class EntityBuilder
        {
        public:
            template <typename Component, typename... ARGS>
            EntityBuilder& Add(ARGS&&... args)
            {
                world_.add<Component>(id_, eastl::forward<ARGS>(args)...);
                return *this;
            }

        private:
            friend World;

            explicit EntityBuilder(World& world) : world_ {world}
            {
                id_ = world_.createEntity();
            };

        private:
            EntityId id_;
            World& world_;
        };*/
}