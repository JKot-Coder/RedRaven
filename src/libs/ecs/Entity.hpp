#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/ComponentTraits.hpp"

namespace RR::Ecs
{
    template <typename ComponentList, typename ArgsTuple>
    struct [[nodiscard]] EntityBuilder;

    template <>
    struct [[nodiscard]] EntityBuilder<void, void>;

    struct Entity
    {
    public:
        EntityId GetId() const { return entity_; }
        void Destruct();
        bool IsAlive() const;
        bool Has(SortedComponentsView componentsView) const;

        template <typename... Components>
        bool Has() const
        {
            eastl::array<ComponentId, sizeof...(Components)> components = {GetComponentId<Components>...};
            eastl::quick_sort(components.begin(), components.end());
            return Has(SortedComponentsView(components));
        }

        EntityBuilder<void, void> Edit();

    private:
        friend World;
        template <typename C, typename A>
        friend struct EntityBuilder;

        Entity(World& world, EntityId entity) : world_(world), entity_(entity) { }

    private:
        World& world_;
        EntityId entity_;
    };
}