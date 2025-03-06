#include "Entity.hpp"

#include "ecs/World.hpp"

namespace RR::Ecs
{
    void Entity::Destruct() { world_.Destruct(entity_); }
    bool Entity::IsAlive() const { return world_.IsAlive(entity_); }
    bool Entity::Has(SortedComponentsView componentsView) const { return world_.Has(entity_, componentsView); }
}