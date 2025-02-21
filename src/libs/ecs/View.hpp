#pragma once

#include "ecs/ComponentTraits.hpp"

namespace RR::Ecs
{
    struct World;

    struct View
    {
        template <typename... Components>
        View Require()
        {
            (require.insert(GetComponentId<Components>), ...);
            return *this;
        }

        template <typename Callable>
        void Each(Callable&& callable) const;

    private:
        friend World;

        View(World& world) : world(world) { };

    public:
        World& world;
        ComponentsSet require;
        ComponentsSet exclude;
    };
}