#pragma once

namespace RR::Ecs
{
    struct World;
    struct Query
    {
        ~Query() { }

        template <typename Callable>
        void Each(Callable&& callable) const;

    private:
        friend World;

        explicit Query(World& world, EntityId id) : world(world), id(id) {};

    private:
        World& world;
        EntityId id;
    };
}
