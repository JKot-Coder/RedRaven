#include "World.hpp"
#include "ecs/EntityBuilder.hpp"
#include "ecs/SystemBuilder.hpp"

namespace
{
    template <typename InputIt1, typename InputIt2, typename Callback>
    void SetDifference(InputIt1 first1, InputIt1 last1,
                       InputIt2 first2, InputIt2 last2, Callback&& clb)
    {
        while (first1 != last1 && first2 != last2)
        {
            if (*first1 < *first2)
                eastl::invoke(eastl::forward<Callback>(clb), *first1++);
            else
            {
                if (!(*first2 < *first1))
                    ++first1;
                ++first2;
            }
        }
    }
}

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

    Archetype& World::getOrCreateArchetype(ArchetypeId archetypeId, SortedComponentsView components)
    {
        Archetype* archetype = nullptr;

        auto it = archetypesMap.find(archetypeId);
        if (it == archetypesMap.end())
        {
            auto archUniqPtr = eastl::make_unique<Archetype>(
                archetypeId,
                ComponentInfoIterator(componentStorage, components.begin()),
                ComponentInfoIterator(componentStorage, components.end()));
            archetype = archUniqPtr.get();
            archetypesMap.emplace(archetypeId, eastl::move(archUniqPtr));

            initCache(*archetype);
        }
        else
            archetype = it->second.get();

        return *archetype;
    }

    void World::initCache(SystemId id)
    {
        cacheForSystemsView.ForEntity(EntityId(id.GetRaw()), [id, this](MatchedArchetypeCache& cache, SystemDescription& systemDesc, Ecs::View& view) {
            for (auto it = archetypesMap.begin(); it != archetypesMap.end(); it++)
            {
                Archetype& archetype = *it->second;
                if (!matches(archetype, view))
                    continue;

                cache.push_back(&archetype);
                for (const auto event : systemDesc.onEvents)
                    archetype.cache[event].push_back(id);
            }

            for (const auto event : systemDesc.onEvents)
                eventsToSystems[event].push_back(id);
        });
    }

    void World::initCache(QueryId id)
    {
        cacheForQueriesView.ForEntity(EntityId(id.GetRaw()), [this](MatchedArchetypeCache& cache, Ecs::View& view) {
            for (auto it = archetypesMap.begin(); it != archetypesMap.end(); it++)
            {
                const Archetype& archetype = *it->second;
                if (!matches(archetype, view))
                    continue;

                cache.push_back(&archetype);
            }
        });
    }

    void World::initCache(Archetype& archetype)
    {
        // This could only happends while world creation process.
        // We create archetype to place this query, and while create archetype we tryng use this query.
        if UNLIKELY (!cacheForQueriesQuery.IsValid())
            return;

        Ecs::Query(*this, cacheForQueriesQuery).ForEach([&archetype](EntityId id, Ecs::View& view, MatchedArchetypeCache& cache, SystemDescription* systemDesc) {
            if (!matches(archetype, view))
                return;

            if (systemDesc)
            {
                for (const auto event : systemDesc->onEvents)
                    archetype.cache[event].push_back(SystemId(id.GetRawId()));
            }

            cache.push_back(&archetype);
        });
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
                unicastEventImmediately(entityId, event);
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
            dispatchEventImmediately({}, systemId, event);
    }

    void World::dispatchEventImmediately(EntityId entity, SystemId systemId, const Ecs::Event& event) const
    {
        cacheForSystemsView.ForEntity(EntityId(systemId.GetRaw()), [&event, entity](World& world, const SystemDescription& desc, MatchedArchetypeCache& cache) {
            desc.onEvent(world, event, entity, RR::Ecs::MatchedArchetypeSpan(cache.begin(), cache.end()));
        });
    }

    void World::unicastEventImmediately(EntityId entity, const Ecs::Event& event) const
    {
        Archetype* from;
        ArchetypeEntityIndex fromIndex;

        if (!ResolveEntityArhetype(entity, from, fromIndex))
            return;

        const auto it = from->cache.find(event.id);
        if (it == from->cache.end())
            return; // Little bit wierd to send event without any subsribers. TODO Maybe log here in bebug

        for (const auto systemId : it->second)
            dispatchEventImmediately(entity, systemId, event);
    }

    Query World::createQuery(Ecs::View&& view)
    {
        Ecs::Entity entt = Entity()
                               .Add<Ecs::View>(eastl::forward<Ecs::View>(view))
                               .Add<MatchedArchetypeCache>()
                               .Apply();

        const auto queryId = QueryId(QueryId::FromValue(entt.GetId().rawId));
        initCache(queryId);

        return Ecs::Query(*this, queryId);
    }

    Ecs::System World::createSystem(SystemDescription&& desc, Ecs::View&& view)
    {
        Ecs::Entity entt = Entity()
                               .Add<Ecs::View>(eastl::forward<Ecs::View>(view))
                               .Add<MatchedArchetypeCache>()
                               .Add<SystemDescription>(eastl::forward<SystemDescription>(desc))
                               .Apply();

        const auto systemId = SystemId(entt.GetId().rawId);
        initCache(systemId);

        return Ecs::System(*this, systemId);
    }

    void World::handleDisappearEvent(EntityId entity, const Archetype& from, const Archetype& to)
    {
        const auto toDissapear = to.cache.find(GetEventId<OnDissapear>);
        const auto fromDissapear = from.cache.find(GetEventId<OnDissapear>);

        if (fromDissapear != from.cache.end())
        {
            if (toDissapear != to.cache.end())
            {
                SetDifference(fromDissapear->second.begin(), fromDissapear->second.end(),
                              toDissapear->second.begin(), toDissapear->second.end(),
                              [entity, this](SystemId systemId) { dispatchEventImmediately(entity, systemId, OnDissapear {}); });
            }
            else
            {
                for (const auto systemId : fromDissapear->second)
                    dispatchEventImmediately(entity, systemId, OnDissapear {});
            }
        }
    }

    void World::handleAppearEvent(EntityId entity, const Archetype* from, const Archetype& to)
    {
        if (!from)
        {
            // First time appear, send On Appear event every subscriber.
            const auto it = to.cache.find(GetEventId<OnAppear>);
            if (it != to.cache.end())
                for (const auto systemId : it->second)
                    dispatchEventImmediately(entity, systemId, OnAppear {});
        }
        else
        {
            const auto toAppear = to.cache.find(GetEventId<OnAppear>);
            const auto fromAppear = from->cache.find(GetEventId<OnAppear>);

            if (toAppear != to.cache.end())
            {
                if (fromAppear != from->cache.end())
                {
                    SetDifference(toAppear->second.begin(), toAppear->second.end(), fromAppear->second.begin(), fromAppear->second.end(), [entity, this](SystemId systemId) { dispatchEventImmediately(entity, systemId, OnAppear {}); });
                }
                else
                    for (const auto systemId : toAppear->second)
                        dispatchEventImmediately(entity, systemId, OnAppear {});
            }
        }
    }
}
