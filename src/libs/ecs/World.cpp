#include "World.hpp"
#include "SystemBuilder.hpp"

namespace RR::Ecs
{
    void World::Tick()
    {
        systemStorage.RegisterDeffered();

        eventStorage.ProcessEvents([this](EntityT entityId, const Ecs::Event& event) {
            if(entityId)
                dispatchEventImmediately(entityId, event);
            else
                broadcastEventImmediately(event);
        });

        //world.progress();
    }

    Entity World::Lookup(const char* name, const char* sep, const char* root_sep, bool recursive) const
    {
        auto e = ecs_lookup_path_w_sep(world, 0, name, sep, root_sep, recursive);
        return Entity(*this, e);
    }

    Entity World::GetAlive(EntityT e) const
    {
        e = ecs_get_alive(world, e);
        return Entity(*this, e);
    }

    void World::broadcastEventImmediately(const Ecs::Event& event) const
    {
        UNUSED(event);
    }

    void World::dispatchEventImmediately(EntityT entity, const Ecs::Event& event) const
    {
        ASSERT(entity);
        UNUSED(entity, event);
    }
}
