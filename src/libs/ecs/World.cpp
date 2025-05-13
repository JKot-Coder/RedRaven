#include "World.hpp"
#include "ecs/ComponentTraits.hpp"
#include "ecs/EntityBuilder.hpp"
#include "ecs/SystemBuilder.hpp"
#include <EASTL/bitvector.h>
#include <EASTL/sort.h>
#include <EASTL/vector_multimap.h>

namespace
{
    template <typename InputIt1, typename InputIt2, typename Callback>
    void ForEachMissingElement(InputIt1 first1, InputIt1 last1,
                               InputIt2 first2, InputIt2 last2, Callback&& clb)
    {
        // This used for systems, which ordered in execution order.
        // So no any optimizations based on numerical order.
        // N^2 complexity, but should be fast enough for small sets.
        // We need this only for Appead Dissapear and in case it could be replaced with cached results.

        while (first1 != last1)
        {
            bool found = false;
            auto it = first2;
            while (it != last2)
            {
                if (*it++ != *first1)
                    continue;

                if(it == first2) first2++;
                if(it == last2) last2--;

                found = true;
                break;
            }

            if (!found)
                eastl::invoke(eastl::forward<Callback>(clb), *first1);

            first1++;
        }
    }

    template <class MarkContainer, class ListContainer, class EdgeContainer, typename LoopDetectedCB>
    bool visitTopSort(uint32_t nodeIdx, const EdgeContainer& edges, MarkContainer& tempMark, MarkContainer& visitedMark, // NOLINT(misc-no-recursion)
                      ListContainer& sortedList, LoopDetectedCB cb)
    {
        if (visitedMark[nodeIdx])
            return true;

        if (tempMark[nodeIdx])
        {
            cb(nodeIdx, tempMark);
            visitedMark.set(nodeIdx, true);
            return false;
        }
        tempMark.set(nodeIdx, true);

        const auto range = edges.equal_range(nodeIdx);
        for (auto edge = range.first; edge != range.second; ++edge)
        {
            if (!visitTopSort(edge->second, edges, tempMark, visitedMark, sortedList, cb))
                return false;
        }

        tempMark.set(nodeIdx, false);
        sortedList.push_back(nodeIdx);
        visitedMark.set(nodeIdx, true);
        return true;
    }

    // https://en.wikipedia.org/wiki/Topological_sorting#Depth-first_search
    template <class ListContainer, class EdgeContainer, typename LoopDetectedCB>
    bool topoSort(uint32_t N, const EdgeContainer& edges, ListContainer& sortedList, LoopDetectedCB cb)
    {
        sortedList.reserve(N);
        eastl::bitvector tempMark(N, false);
        eastl::bitvector visitedMark(N, false);
        for (uint32_t i = 0; i < N; ++i)
        {
            if (!visitTopSort(i, edges, tempMark, visitedMark, sortedList, cb))
                return false;
        }
        return true;
    }
}

namespace RR::Ecs
{
    World::World() : creationThreadID(std::this_thread::get_id()), queriesView(*this), systemsView(*this)
    {
        RegisterComponent<EntityId>();
        RegisterComponent<Ecs::View>();
        RegisterComponent<MatchedArchetypeCache>();

        {
            // Manually create archetype for queries
            // This is required because of cyclic dependency: creating arhetype require queriesQuery and
            // creating queriesQuery query require creating archetype
            constexpr eastl::array<ComponentId, 3> components = {GetComponentId<EntityId>, GetComponentId<Ecs::View>, GetComponentId<MatchedArchetypeCache>};
            static_assert(components[0] < components[1]);
            static_assert(components[1] < components[2]);

            ArchetypeId archetypeId = GetArchetypeIdForComponents(SortedComponentsView(components));
            Archetype& archetype = createArchetypeNoCache(archetypeId, SortedComponentsView(components));
            archetypesCache.push_back(&archetype);
        }

        queriesQuery = Query().Require<Ecs::View, MatchedArchetypeCache>().Exclude<SystemDescription>().Build().id;
        systemsQuery = Query().Require<Ecs::View, SystemDescription, MatchedArchetypeCache>().Build().id;
        queriesView.Require<Ecs::View, MatchedArchetypeCache>();
        systemsView.Require<Ecs::View, SystemDescription, MatchedArchetypeCache>();
    }

    void World::Destroy(EntityId entityId)
    {
        ASSERT_IS_CREATION_THREAD;
        if (!IsAlive(entityId)) return;

        if (IsLocked())
        {
            entityStorage.PendingDestroy(entityId);
            commandBuffer.Destroy(entityId);
        }
        else
            destroyImpl(entityId);
    }

    Ecs::EntityBuilder<void, void> World::Entity()
    {
        ASSERT_IS_CREATION_THREAD;
        return EntityBuilder<void, void>(*this, {});
    }

    Ecs::Entity World::EmptyEntity()
    {
        ASSERT_IS_CREATION_THREAD;
        EntityId entityId = commit<TypeList<>>({}, {nullptr, nullptr}, std::make_tuple(), eastl::make_index_sequence<0>());
        return Ecs::Entity(*this, entityId);
    }

    SystemBuilder World::System() { return Ecs::SystemBuilder(*this); }
    SystemBuilder World::System(const HashName& name) { return Ecs::SystemBuilder(*this, name); }

    void World::RunSystem(SystemId systemId) const
    {
        ASSERT_IS_CREATION_THREAD;
        systemsView.ForEntity(EntityId(systemId.GetRaw()), [](World& world, const SystemDescription& desc, MatchedArchetypeCache& cache) {
            desc.callback(world, nullptr, {}, RR::Ecs::MatchedArchetypeSpan(cache.begin(), cache.end()));
        });
    }

    void World::OrderSystems()
    {
        ASSERT_IS_CREATION_THREAD;
        ASSERT(!IsLocked());

        if (!systemsOrderDirty)
            return;

        struct SystemHandle
        {
            SystemHandle(const SystemDescription& desc, SystemId id, HashName hashName)
                : desc(&desc), id(id), hashName(std::move(hashName)) { }

        public:
            const SystemDescription* desc;
            SystemId id;
            HashName hashName;
        };

        eastl::vector<SystemHandle> tmpSystemList;
        Ecs::Query(*this, systemsQuery).ForEach([&tmpSystemList](EntityId id, const SystemDescription& desc, const HashName& hashName) {
            tmpSystemList.emplace_back(desc, SystemId(id.GetRawId()), hashName);
        });

        // Sort by id to avoid depending on native ES registration order
        // which might be different on different platforms, depend on hot-reload, etc...
        eastl::sort(tmpSystemList.begin(), tmpSystemList.end(), [](auto a, auto b) { return a.hashName < b.hashName; });

        // Map hash of name to more simple global index
        absl::flat_hash_map<HashType, uint32_t> systemHashToIdxMap;

        for (uint32_t i = 0; i < tmpSystemList.size(); i++)
            systemHashToIdxMap[tmpSystemList[i].hashName.hash] = i;

        constexpr uint32_t invalidId = std::numeric_limits<uint32_t>::max();

        auto systemHashToIdx = [&systemHashToIdxMap](HashType hash) {
            const auto it = systemHashToIdxMap.find(hash);
            return it != systemHashToIdxMap.end() ? it->second : invalidId;
        };

        eastl::vector_multimap<uint32_t, uint32_t> edges;
        auto makeEdge = [&](uint32_t fromIdx, uint32_t toIdx) { edges.emplace(fromIdx, toIdx); };

        auto insertOrderEdge = [&](const HashName& system, const HashName& other, bool before) {
            const auto systemIdx = systemHashToIdx(system.hash);
            const auto otherIdx = systemHashToIdx(other.hash);
            ASSERT(systemIdx != invalidId); // Impossible

            if UNLIKELY (otherIdx == invalidId)
            {
                Log::Format::Error("ES <{}> is supposed to be {} ES <{}>, which is undeclared.", system.string.c_str(),
                                   before ? "before" : "after", other.string.c_str());
                return;
            }

            makeEdge(before ? otherIdx : systemIdx, before ? systemIdx : otherIdx);
        };

        // Build edges based on before/after
        for (const SystemHandle& handle : tmpSystemList)
        {
            for (const auto& other : handle.desc->after)
                insertOrderEdge(handle.hashName, other, false);

            for (const auto& other : handle.desc->before)
                insertOrderEdge(handle.hashName, other, true);
        }

        eastl::vector<uint32_t> sortedList;
        auto loopDetected = [&](size_t idx, auto&) {
            Log::Format::Error("ES <{}> in graph to become cyclic and was removed from sorting. ES order is non-deterministic.",
                               tmpSystemList[idx].hashName.string.c_str());
        };
        topoSort(static_cast<uint32_t>(tmpSystemList.size()), edges, sortedList, loopDetected);

        absl::flat_hash_map<SystemId, uint32_t> systemsOrder;
        systemsOrder.reserve(tmpSystemList.size());
        for (size_t i = 0; i < sortedList.size(); ++i)
        {
            const auto& system = tmpSystemList[sortedList[i]];
            systemsOrder[system.id] = static_cast<uint32_t>(i);
        }

        for (auto& eventSubscribersPair : eventSubscribers)
            eastl::sort(eventSubscribersPair.second.begin(), eventSubscribersPair.second.end(), [&systemsOrder](auto a, auto b) { return systemsOrder[a] < systemsOrder[b]; });

        for (auto* archetype : archetypesCache)
        {
            for (auto& cache : archetype->cache)
                eastl::sort(cache.second.begin(), cache.second.end(), [&systemsOrder](auto a, auto b) { return systemsOrder[a] < systemsOrder[b]; });
        }
        {
            // clang-format off
            // This is a hacky way to sort systems in archetype storage.
            // We temporarily remove all systems from archetype storage and reinsert them in the desired order.
            // This approach heavily relies on internal details of the Archetype implementation.
            // However, it significantly reduces sorting overhead at runtime during cache filling, as it ensures systems are already sorted.
            // Sorting speed is not critical since this should ideally run only once at startup.
            struct OrderTag{}; // clang-format on
            for (const SystemHandle& handle : tmpSystemList)
                Ecs::Entity(*this, EntityId(handle.id.GetRaw())).Edit().Add<OrderTag>().Apply();

            for (const auto sortedIndex : sortedList)
                Ecs::Entity(*this, EntityId(tmpSystemList[sortedIndex].id.GetRaw())).Edit().Remove<OrderTag>().Apply();
        }

        systemsOrderDirty = false;
    }

    void World::ProcessDefferedEvents()
    {
        ASSERT_IS_CREATION_THREAD;
        ASSERT(!IsLocked());

        eventStorage.ProcessEvents([this](EntityId entityId, const Ecs::Event& event) {
            if (entityId)
                unicastEventImmediately(entityId, event);
            else
                broadcastEventImmediately(event);
        });
    }

    void World::Tick()
    {
        ASSERT_IS_CREATION_THREAD;
        ASSERT(!IsLocked());
        OrderSystems();
        // Update events;

        ProcessDefferedEvents();
    }

    void World::destroyImpl(EntityId entityId)
    {
        ASSERT_IS_CREATION_THREAD;
        ASSERT(!IsLocked());

        EntityRecord record;
        if (ResolveEntityRecord(entityId, record))
        {
            unicastEventImmediately(entityId, OnDissapear {});
            record.GetArchetype(false)->Delete(entityStorage, record.GetIndex(false), false);
        }

        entityStorage.Destroy(entityId);
    }

    Ecs::System World::createSystem(SystemDescription&& desc, Ecs::View&& view, HashName&& name)
    {
        ASSERT_IS_CREATION_THREAD;
        Ecs::Entity entt = Entity()
                               .Add<Ecs::View>(eastl::forward<Ecs::View>(view))
                               .Add<MatchedArchetypeCache>()
                               .Add<SystemDescription>(eastl::forward<SystemDescription>(desc))
                               .Add<HashName>(eastl::forward<HashName>(name))
                               .Apply();

        const auto systemId = SystemId(entt.GetId().rawId);
        initCache(systemId);

        systemsOrderDirty = true;

        return Ecs::System(*this, systemId);
    }

    Query World::createQuery(Ecs::View&& view)
    {
        ASSERT_IS_CREATION_THREAD;
        Ecs::Entity entt = Entity()
                               .Add<Ecs::View>(eastl::forward<Ecs::View>(view))
                               .Add<MatchedArchetypeCache>()
                               .Apply();

        const auto queryId = QueryId(QueryId::FromValue(entt.GetId().rawId));
        initCache(queryId);

        return Ecs::Query(*this, queryId);
    }

    void World::initCache(SystemId id)
    {
        // TODO NOT ALLOW FOR deffered mode!!!!!
        systemsView.ForEntity(EntityId(id.GetRaw()), [id, this](MatchedArchetypeCache& cache, SystemDescription& systemDesc, Ecs::View& view) {
            for (auto* archetype : archetypesCache)
            {
                if LIKELY (!matches(*archetype, view))
                    continue;

                cache.push_back(archetype);
                for (const auto event : systemDesc.onEvents)
                    archetype->cache[event].push_back(id);
            }

            for (const auto event : systemDesc.onEvents)
                eventSubscribers[event].push_back(id);
        });
    }

    void World::initCache(QueryId id)
    {
        // TODO NOT ALLOW FOR deffered mode!!!!!
        queriesView.ForEntity(EntityId(id.GetRaw()), [this](MatchedArchetypeCache& cache, Ecs::View& view) {
            for (const auto* archetype : archetypesCache)
            {
                if LIKELY(!matches(*archetype, view))
                    continue;

                cache.push_back(archetype);
            }
        });
    }

    void World::initCache(Archetype& archetype)
    {
        ASSERT_IS_CREATION_THREAD;
        ASSERT(!IsLocked());

        archetypesCache.push_back(&archetype);

        Ecs::Query(*this, queriesQuery).ForEach([&archetype](Ecs::View& view, MatchedArchetypeCache& cache) {
            if LIKELY (!matches(archetype, view))
                return;

            cache.push_back(&archetype);
        });

        Ecs::Query(*this, systemsQuery).ForEach([&archetype](EntityId id, Ecs::View& view, MatchedArchetypeCache& cache, SystemDescription& systemDesc) {
            if LIKELY (!matches(archetype, view))
                return;

            cache.push_back(&archetype);

            for (const auto event : systemDesc.onEvents)
                archetype.cache[event].push_back(SystemId(id.GetRawId()));
        });
    }

    Archetype& World::createArchetypeNoCache(ArchetypeId archetypeId, SortedComponentsView components)
    {
        ASSERT_IS_CREATION_THREAD;

        auto* archetype = archetypesMap.emplace(archetypeId,
                                                eastl::make_unique<Archetype>(
                                                    ComponentInfoIterator(componentStorage, components.begin()),
                                                    ComponentInfoIterator(componentStorage, components.end())))
                              .first->second.get();

        return *archetype;
    }

    Archetype& World::getOrCreateArchetype(ArchetypeId archetypeId, SortedComponentsView components)
    {
        ASSERT_IS_CREATION_THREAD;
        Archetype* archetype = nullptr;

        auto it = archetypesMap.find(archetypeId);
        if (it == archetypesMap.end())
        {
            archetype = &createArchetypeNoCache(archetypeId, components);

            if (IsLocked())
                commandBuffer.InitCache(*archetype);
            else
                initCache(*archetype);
        }
        else
            archetype = it->second.get();

        return *archetype;
    }

    void World::handleDisappearEvent(EntityId entity, const Archetype& from, const Archetype& to)
    {
        ASSERT_IS_CREATION_THREAD;
        const auto toDissapear = to.cache.find(GetEventId<OnDissapear>);
        const auto fromDissapear = from.cache.find(GetEventId<OnDissapear>);

        if (fromDissapear != from.cache.end())
        {
            if (toDissapear != to.cache.end())
            {
                ForEachMissingElement(fromDissapear->second.begin(), fromDissapear->second.end(),
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
        ASSERT_IS_CREATION_THREAD;
        if (from == nullptr)
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
                    ForEachMissingElement(toAppear->second.begin(), toAppear->second.end(),
                                          fromAppear->second.begin(), fromAppear->second.end(),
                                          [entity, this](SystemId systemId) { dispatchEventImmediately(entity, systemId, OnAppear {}); });
                }
                else
                    for (const auto systemId : toAppear->second)
                        dispatchEventImmediately(entity, systemId, OnAppear {});
            }
        }
    }

    void World::broadcastEventImmediately(const Ecs::Event& event) const
    {
        ASSERT_IS_CREATION_THREAD;
        ASSERT(!systemsOrderDirty);

        const auto it = eventSubscribers.find(event.id);
        if (it == eventSubscribers.end())
            return;

        for (const auto systemId : it->second)
            dispatchEventImmediately({}, systemId, event);
    }

    void World::dispatchEventImmediately(EntityId entity, SystemId systemId, const Ecs::Event& event) const
    {
        ASSERT_IS_CREATION_THREAD;
        ASSERT(!systemsOrderDirty);

        systemsView.ForEntity(EntityId(systemId.GetRaw()), [&event, entity](World& world, const SystemDescription& desc, MatchedArchetypeCache& cache) {
            desc.callback(world, &event, entity, RR::Ecs::MatchedArchetypeSpan(cache.begin(), cache.end()));
        });
    }

    void World::unicastEventImmediately(EntityId entity, const Ecs::Event& event) const
    {
        ASSERT_IS_CREATION_THREAD;
        ASSERT(!systemsOrderDirty);

        EntityRecord record;
        if (!ResolveEntityRecord(entity, record))
            return;

        // TODO. Any pending mutations/deletion will be ignored. Add message/asset about it?
        const auto it = record.GetArchetype(false)->cache.find(event.id);
        if (it == record.GetArchetype(false)->cache.end())
            return;

        for (const auto systemId : it->second)
            dispatchEventImmediately(entity, systemId, event);
    }
}
