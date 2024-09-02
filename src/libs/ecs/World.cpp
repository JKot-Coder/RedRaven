#include "World.hpp"

namespace RR::Ecs
{
    void world::tick()
    {
        eventStorage.ProcessEvents([this](entity_t entityId, const Ecs::event& event) {
            if(entityId)
                dispatchEventImmediately(entityId, event);
            else
                broadcastEventImmediately(event);
        });

        //world.progress();
    }

    void world::broadcastEventImmediately(const Ecs::event& event) const
    {
        UNUSED(event);
    }

    void world::dispatchEventImmediately(entity_t entity, const Ecs::event& event) const
    {
        ASSERT(entity);
        UNUSED(entity, event);
    }
}
