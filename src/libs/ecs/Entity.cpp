#include "Entity.hpp"

#include "ecs/World.hpp"
#include "ecs/Archetype.hpp"

namespace RR::Ecs
{
    void Entity::Destruct() const { world_.Destruct(entity_); }
    bool Entity::IsAlive() const { return world_.IsAlive(entity_); }
    bool Entity::Has(SortedComponentsView componentsView) const { return world_.Has(entity_, componentsView); }
    bool Entity::ResolveEntityArhetype(Archetype*& archetype, ArchetypeEntityIndex& index) const
    {
        return world_.ResolveEntityArhetype(entity_, archetype, index);
    }
}