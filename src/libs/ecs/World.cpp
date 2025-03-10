#include "World.hpp"
#include "ecs/SystemBuilder.hpp"
#include "ecs/EntityBuilder.hpp"

namespace RR::Ecs
{
    Entity World::Entity()
    {
        return Ecs::Entity(*this, createEntity());
    }

    SystemBuilder World::System() { return Ecs::SystemBuilder(*this); }

    Entity World::Entity(EntityId entityId)
    {
        ASSERT(IsAlive(entityId));
        return Ecs::Entity(*this, entityId);
    }

    void World::Tick()
    {
        //systemStorage.RegisterDeffered();

        eventStorage.ProcessEvents([this](EntityId entityId, const Ecs::Event& event) {
            if (entityId)
                dispatchEventImmediately(entityId, event);
            else
                broadcastEventImmediately(event);
        });

        // world.progress();
    }
    /*
        Entity World::Lookup(const char* name, const char* sep, const char* root_sep, bool recursive) const
        {
            auto e = ecs_lookup_path_w_sep(world, 0, name, sep, root_sep, recursive);
            return Entity(*this, e);
        }

        Entity World::GetAlive(EntityId e) const
        {
            e = ecs_get_alive(world, e);
            return Entity(*this, e);
        }*/

    void World::broadcastEventImmediately(const Ecs::Event& event) const
    {
        const auto it = eventsToSystems.find(event.id);
        ASSERT(it != eventsToSystems.end()); // TODO should be ok btw

      //  for(const auto systemId : it->second)
      //      systems[systemId.Value()].onEvent(event, );

        UNUSED(event);
    }

    void World::dispatchEventImmediately(EntityId entity, const Ecs::Event& event) const
    {
        ASSERT(entity);
        UNUSED(entity, event);
    }
}
