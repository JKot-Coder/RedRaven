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
        void Each(Callable&& callable) const;

    private:
        friend World;

        explicit Query(World& world, QueryId id) : world(world), id(id) {};

    private:
        World& world;
        QueryId id;
    };
}
