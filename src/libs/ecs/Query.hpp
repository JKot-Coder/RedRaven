#pragma once

#include "ecs/View.hpp"
#include "ecs/Index.hpp"

namespace RR::Ecs
{
    struct World;
    using QueryId = Index<struct QueryIdTag, size_t>;

    struct Query final
    {
        ~Query() { }

        template <typename Callable>
        void ForEach(Callable&& callable) const;

    private:
        friend World;
        friend QueryBuilder;

        explicit Query(World& world, QueryId id) : world(world), id(id) {};

    private:
        World& world;
        QueryId id;
    };

    struct QueryBuilder final
    {
        ~QueryBuilder() { }

        template <typename... Components>
        QueryBuilder Require() &&
        {
            view.Require<Components...>();
            return *this;
        }

        template <typename... Components>
        QueryBuilder Exclude() &&
        {
            view.Exclude<Components...>();
            return *this;
        }

        Query Build() &&;

    private:
        friend World;

        explicit QueryBuilder(World& world) : view(world) {};

    private:
        View view;
    };
}
