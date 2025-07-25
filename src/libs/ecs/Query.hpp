#pragma once

#include "ecs/Index.hpp"
#include "ecs/View.hpp"

namespace RR::Ecs
{
    struct World;
    using QueryId = Index<struct QueryIdTag, uint32_t>;

    struct Query final
    {
        ~Query() { }

        template <typename Callable>
        void ForEach(Callable&& callable) const;

    private:
        friend World;
        friend QueryBuilder;
        friend SystemBuilder; // TODO don't think it's should be here;

        explicit Query(World& world, QueryId id) : world(world), id(id) { };

    private:
        World& world;
        QueryId id;
    };

    struct QueryBuilder final
    {
        ~QueryBuilder() { }

        template <typename... Components>
        QueryBuilder With() &&
        {
            view.With<Components...>();
            return *this;
        }

        template <typename... Components>
        QueryBuilder Without() &&
        {
            view.Without<Components...>();
            return *this;
        }

        Query Build() &&;

    private:
        friend World;

        explicit QueryBuilder(World& world) : view(world) { };

    private:
        View view;
    };
}
