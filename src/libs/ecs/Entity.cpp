#include "Entity.hpp"

#include "ecs/World.hpp"
#include "ecs/Archetype.hpp"

namespace RR::Ecs
{
    void Entity::Destruct() const { world->Destruct(id); }
    bool Entity::IsAlive() const { return world->IsAlive(id); }
    bool Entity::Has(SortedComponentsView componentsView) const { return world->Has(id, componentsView); }
    bool Entity::ResolveArhetype(Archetype*& archetype, ArchetypeEntityIndex& index) const
    {
        return world->ResolveEntityArhetype(id, archetype, index);
    }
}