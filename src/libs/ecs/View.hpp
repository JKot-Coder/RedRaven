#pragma once

#include "ecs/ComponentTraits.hpp"

namespace RR::Ecs
{
    struct View
    {
        template <typename... Components>
        View Require()
        {
            (require.insert(GetComponentId<Components>), ...);
            return *this;
        }

        template <typename... Components>
        View Exclude()
        {
            (exclude.insert(GetComponentId<Components>), ...);
            return *this;
        }

        template <typename Callable>
        void ForEach(Callable&& callable) const;

    private:
        friend struct World;
        friend struct SystemBuilder;
        friend struct QueryBuilder;

        View(World& world) : world(world) { };

    public:
        World& world;
        ComponentsSet require;
        ComponentsSet exclude;
    };
}