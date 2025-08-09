#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/meta/ComponentTraits.hpp"

#include <EASTL/sort.h>

namespace RR::Ecs
{
    template <typename ComponentList, typename ArgsTuple>
    struct [[nodiscard]] EntityBuilder;

    template <>
    struct [[nodiscard]] EntityBuilder<void, void>;

    struct Entity
    {
    public:
        void Destroy() const;

        [[nodiscard]] EntityId GetId() const { return id; }
        [[nodiscard]] bool IsAlive() const;
        [[nodiscard]] bool Has(SortedComponentsView componentsView) const;
        [[nodiscard]] bool ResolveArhetype(Archetype*& archetype, ArchetypeEntityIndex& index) const;

        template <typename... Components>
        [[nodiscard]] bool Has() const
        {
            eastl::array<ComponentId, sizeof...(Components)> components = {GetComponentId<Components>...};
            eastl::sort(components.begin(), components.end());
            return Has(SortedComponentsView(components));
        }

        EntityBuilder<void, void> Edit() const;

    private:
        friend World;
        template <typename C, typename A>
        friend struct EntityBuilder;

        Entity(World& world, EntityId id) : world(&world), id(id) { ASSERT(id); }

    private:
        World* world;
        EntityId id;
    };
}