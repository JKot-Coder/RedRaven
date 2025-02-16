#include "World.hpp"
#include "SystemBuilder.hpp"
#include "ecs\EntityBuilder.hpp"

namespace RR::Ecs
{
    EntityBuilder<void, void>World::Entity()
    {
        return EntityBuilder<void, void>(*this);
    }

    void World::Tick()
    {
        systemStorage.RegisterDeffered();

        /*eventStorage.ProcessEvents([this](EntityId entityId, const Ecs::Event& event) {
            if(entityId)
                dispatchEventImmediately(entityId, event);
            else
                broadcastEventImmediately(event);
        });*/
        //world.progress();
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
    }

    void World::broadcastEventImmediately(const Ecs::Event& event) const
    {
        UNUSED(event);
    }

    void World::dispatchEventImmediately(EntityId entity, const Ecs::Event& event) const
    {
        ASSERT(entity);
        UNUSED(entity, event);
    }*/
}
