#pragma once

#include "ecs/Entity.hpp"
#include "ecs/ComponentTraits.hpp"

namespace RR::Ecs
{
    struct View
    {
        template <typename... Components>
        View With()
        {
            (with.insert(GetComponentId<Components>), ...);
            return *this;
        }

        template <typename... Components>
        View Without()
        {
            (without.insert(GetComponentId<Components>), ...);
            return *this;
        }

        template <typename Callable>
        void ForEach(Callable&& callable) const;
        template <typename Callable>
        void ForEntity(EntityId id, Callable&& callable) const;
        template <typename Callable>
        void ForEntity(Entity entity, Callable&& callable) const;

        const ComponentsSet& GetWithSet() const { return with; }
        const ComponentsSet& GetWithoutSet() const { return without; }

    private:
        friend struct World;
        friend struct SystemBuilder;
        friend struct QueryBuilder;

        View(World& world) : world(world) { };

    public:
        World& world;
        ComponentsSet with;
        ComponentsSet without;
    };
}