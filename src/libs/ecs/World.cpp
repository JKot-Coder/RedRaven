#include "World.hpp"
#include "ecs/EntityBuilder.hpp"
#include "ecs/SystemBuilder.hpp"

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

    Ecs::EntityBuilder<void, void> World::Entity()
    {
        return EntityBuilder<void, void>(*this, {});
    }

    Ecs::Entity World::EmptyEntity()
    {
        EntityId entityId = commit<TypeList<>>({}, SortedComponentsView {nullptr, nullptr}, std::make_tuple());
        return Ecs::Entity(*this, entityId);
    }

    Ecs::Entity World::GetEntity(EntityId entityId)
    {
        return Ecs::Entity(*this, entityId);
    }

    SystemBuilder World::System() { return Ecs::SystemBuilder(*this); }

    void World::ProcessDefferedEvents()
    {
        eventStorage.ProcessEvents([this](EntityId entityId, const Ecs::Event& event) {
            if (entityId)
                dispatchEventImmediately(entityId, event);
            else
                broadcastEventImmediately(event);
        });
    }

    void World::Tick()
    {
        ProcessDefferedEvents();
        // systemStorage.RegisterDeffered();

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
        if (it == eventsToSystems.end())
            return;

        for (const auto systemId : it->second)
        {
            cacheForSystemsView.ForEntity(EntityId(systemId.GetRaw()), [&event](World& world, const SystemDescription& desc, MatchedArchetypeCache& cache) {
                desc.onEvent(world, event, {}, RR::Ecs::MatchedArchetypeSpan(cache.begin(), cache.end()));
            });
        }
    }

    void World::dispatchEventImmediately(EntityId entity, const Ecs::Event& event) const
    {
        Archetype* from;
        ArchetypeEntityIndex fromIndex;

        if(!ResolveEntityArhetype(entity, from, fromIndex))
            return;

        const auto it = from->cache.find(event.id);
        if(it == from->cache.end())
            return;    // Little bit wierd to send event without any subsribers. TODO Maybe log here in bebug

        for (const auto systemId : it->second)
        {
            cacheForSystemsView.ForEntity(EntityId(systemId.GetRaw()), [&event, entity](World& world, const SystemDescription& desc, MatchedArchetypeCache& cache) {
                desc.onEvent(world, event, entity, RR::Ecs::MatchedArchetypeSpan(cache.begin(), cache.end()));
            });
        }

      /*/  const auto it = eventsToSystems.find(event.id);
        // Little bit wierd to send event without any subsribers. TODO Maybe log here in bebug
        if (it == eventsToSystems.end())
            return;

        for (const auto systemId : it->second)
        {
            cacheForSystemsView.ForEntity(EntityId(systemId.GetRaw()), [&event](World& world, const SystemDescription& desc, MatchedArchetypeCache& cache) {
                if(matches(entity, desc.),  ))
                    continue;
                

                desc.onEvent(world, event, RR::Ecs::MatchedArchetypeSpan(cache.begin(), cache.end()));
            });
        }
        queryForEntity(entity, *this, eastl::forward<Callable>(callable));*/

    }

    QueryId World::_register(const Ecs::View& view)
    {
        Ecs::Entity entt = Entity()
                               .Add<Ecs::View>(view)
                               .Add<MatchedArchetypeCache>()
                               .Apply();

        cacheForQueriesView.ForEntity(entt, [this, &view](MatchedArchetypeCache& cache) {
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

        Ecs::Entity entt = Entity()
                               .Add<Ecs::View>(eastl::forward<Ecs::View>(view))
                               .Add<MatchedArchetypeCache>()
                               .Add<SystemDescription>(eastl::forward<SystemDescription>(desc))
                               .Apply();

        const auto systemId = SystemId(entt.GetId().rawId);

        updateCache(systemId);

        return Ecs::System(*this, systemId);
    }
}
