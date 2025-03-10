#include "World.hpp"
#include "ecs/SystemBuilder.hpp"
#include "ecs/EntityBuilder.hpp"

namespace RR::Ecs
{
    World::World() : cacheForQueriesView(*this), cacheForSystemsView(*this)
    {
        RegisterComponent<EntityId>();
        RegisterComponent<Ecs::View>();
        RegisterComponent<MatchedArchetypeCache>();

        {
            // Manually create archetype for queries
            // This is required because of cyclic dependency of creating arhetype require cacheForQueriesQuery and
            // creating cacheForQueriesQuery query require creating archetype
            constexpr eastl::array<ComponentId, 3> components = {GetComponentId<EntityId>, GetComponentId<Ecs::View>, GetComponentId<MatchedArchetypeCache>};
            static_assert(components[0] < components[1]);
            static_assert(components[1] < components[2]);

            ArchetypeId archetypeId = GetArchetypeIdForComponents(SortedComponentsView(components));
            getOrCreateArchetype(archetypeId, SortedComponentsView(components));
        }

        cacheForQueriesQuery = Query().Require<Ecs::View, MatchedArchetypeCache>().Build().id;
        cacheForQueriesView.Require<Ecs::View, MatchedArchetypeCache>();
        cacheForSystemsView.Require<Ecs::View, SystemDescription, MatchedArchetypeCache>();
    }

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
        // Little bit wierd to send event without any subsribers. TODO Maybe log here in bebug
        if(it == eventsToSystems.end())
            return;

        for(const auto systemId : it->second)
    {
            cacheForSystemsView.ForEntity(EntityId(systemId.Value()), [&event](World& world, const SystemDescription& desc, MatchedArchetypeCache& cache) {
                desc.onEvent(world, event, RR::Ecs::MatchedArchetypeSpan(cache.begin(), cache.end()));
            });
        }

        UNUSED(event);
    }

    void World::dispatchEventImmediately(EntityId entity, const Ecs::Event& event) const
    {
        ASSERT(entity);
        UNUSED(entity, event);
    }

    QueryId World::_register(const Ecs::View& view)
    {
        Ecs::Entity entt = Entity();
        entt.Edit()
            .Add<Ecs::View>(view)
            .Add<MatchedArchetypeCache>()
            .Apply();

        cacheForQueriesView.ForEntity(entt, [this, &view](MatchedArchetypeCache& cache){
            for (auto it = archetypesMap.begin(); it != archetypesMap.end(); it++)
            {
                const Archetype& archetype = *it->second;
                if (!matches(archetype, view))
                    continue;

                cache.push_back(&archetype);
            }
        });

        return QueryId::FromValue(entt.GetId().rawId);
    }

    Ecs::System World::Create(SystemDescription&& desc, Ecs::View&& view)
    {
        UNUSED(desc);
        Ecs::Entity entt = Entity();
        entt.Edit()
            .Add<Ecs::View>(eastl::forward<Ecs::View>(view))
            .Add<MatchedArchetypeCache>()
            .Add<SystemDescription>(eastl::forward<SystemDescription>(desc))
            .Apply();

        cacheForQueriesView.ForEntity(entt, [this, &view](MatchedArchetypeCache& cache){
            for (auto it = archetypesMap.begin(); it != archetypesMap.end(); it++)
            {
                const Archetype& archetype = *it->second;
                if (!matches(archetype, view))
                    continue;

                cache.push_back(&archetype);
            }
        });

        const auto systemId = SystemId(entt.GetId().rawId);
        for (const auto event : desc.onEvents)
            eventsToSystems[event].push_back(systemId);

        return Ecs::System(*this, systemId);
    }
}
