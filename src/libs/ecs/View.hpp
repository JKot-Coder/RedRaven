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
            auto check = [&]([[maybe_unused]] auto id, [[maybe_unused]] auto name) {
                ECS_VERIFY(without.find(id) == without.end(), "Component {} is already in without.", name);
            };
            (check(GetComponentId<Components>, GetComponentName<Components>), ...);

            (with.insert(GetComponentId<Components>), ...);
            return *this;
        }

        template <typename... Components>
        View Without()
        {
            static_assert((!IsSingleton<Components>&&...), "Singleton components cannot be used in Without filtering");
            auto check = [&]([[maybe_unused]] auto id, [[maybe_unused]] auto name) {
                ECS_VERIFY(with.find(id) == with.end(), "Component {} is already in with.", name);
            };
            (check(GetComponentId<Components>, GetComponentName<Components>), ...);

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

        World& world;
        ComponentsSet with;
        ComponentsSet without;
    };
}