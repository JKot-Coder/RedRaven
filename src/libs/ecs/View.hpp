#pragma once

#include "ecs/Entity.hpp"
#include "ecs/meta/ComponentTraits.hpp"

namespace RR::Ecs
{
    struct View
    {
        template <typename... Components>
        View With()
        {
            auto check = [&]([[maybe_unused]] auto id, [[maybe_unused]] auto name) {
                ECS_VERIFY(without.find(id) == without.end(), "Component {} is already in without.", name);
            };
            (check(Meta::GetComponentId<Components>, Meta::GetComponentName<Components>), ...);

            (with.insert(Meta::GetComponentId<Components>), ...);
            return *this;
        }

        template <typename... Components>
        View Without()
        {
            auto check = [&]([[maybe_unused]] auto id, [[maybe_unused]] auto name) {
                ECS_VERIFY(with.find(id) == with.end(), "Component {} is already in with.", name);
            };
            (check(Meta::GetComponentId<Components>, Meta::GetComponentName<Components>), ...);

            (without.insert(Meta::GetComponentId<Components>), ...);
            return *this;
        }

        template <typename Callable>
        void ForEach(Callable&& callable) const;
        template <typename Callable>
        void ForEntity(EntityId id, Callable&& callable) const;
        template <typename Callable>
        void ForEntity(Entity entity, Callable&& callable) const;

        const Meta::ComponentsSet& GetWithSet() const { return with; }
        const Meta::ComponentsSet& GetWithoutSet() const { return without; }

    private:
        friend struct World;
        friend struct SystemBuilder;
        friend struct QueryBuilder;

        View(World& world) : world(world) { };

        World& world;
        Meta::ComponentsSet with;
        Meta::ComponentsSet without;
    };
}