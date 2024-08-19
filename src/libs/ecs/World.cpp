#include "World.hpp"

namespace RR::Ecs
{
    void World::Tick()
    {
        eventStorage.ProcessEvents([this](EntityId entityId, const Ecs::Event& event) {
            if(entityId)
                dispatchEventImmediately(entityId, event);
            else
                broadcastEventImmediately(event);
        });

        world.progress();
    }

    void World::broadcastEventImmediately(const Ecs::Event& event) const
    {
        UNUSED(event);
    }

    void World::dispatchEventImmediately(EntityId entity, const Ecs::Event& event) const
    {
        ASSERT(entity);
        UNUSED(entity, event);
    }
}
