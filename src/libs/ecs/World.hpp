#pragma once

#include "ecs/Archetype.hpp"
#include "ecs/ComponentStorage.hpp"
#include "ecs/Entity.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/EntityStorage.hpp"
#include "ecs/Event.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/FunctionTraits.hpp"
#include "ecs/IterationHelpers.hpp"
#include "ecs/Query.hpp"
#include "ecs/System.hpp"
#include "ecs/View.hpp"
#include "ska/flat_hash_map.h"

namespace RR::Ecs
{
    struct World
    {
    private:
        struct QueryData
        {
            View view;
            eastl::fixed_vector<const Archetype*, 16> cache;
        };

        using MatchedArchetypeCache = eastl::fixed_vector<const Archetype*, 16>; // TODO naming?

    public:
        SystemBuilder System();
        Ecs::EntityBuilder<void, void> Entity();
        Ecs::Entity EmptyEntity();
        Ecs::Entity GetEntity(EntityId entityId);

        Ecs::View View() { return Ecs::View(*this); }
        Ecs::QueryBuilder Query() { return Ecs::QueryBuilder(*this); }

        Ecs::System Create(SystemDescription&& desc, Ecs::View&& view);

        bool IsAlive(EntityId entityId) const { return entityStorage.IsAlive(entityId); }

        bool Has(EntityId entityId, SortedComponentsView components)
        {
            Archetype* archetype = nullptr;
            ArchetypeEntityIndex index;

            if (ResolveEntityArhetype(entityId, archetype, index))
                return archetype->HasAll(components);

            return false;
        }

        void Destruct(EntityId entityId)
        {
            if (!IsAlive(entityId)) return;

            Archetype* archetype = nullptr;
            ArchetypeEntityIndex index;
            if (ResolveEntityArhetype(entityId, archetype, index))
                archetype->Delete(entityStorage, index, false);

            entityStorage.Destroy(entityId);
        }

        template <typename Component>
        ComponentId RegisterComponent() { return componentStorage.Register<Component>(); }

        bool ResolveEntityArhetype(EntityId entity, Archetype*& archetype, ArchetypeEntityIndex& index) const
        {
            EntityRecord record;
            if (!entityStorage.Get(entity, record))
                return false;

            ASSERT(record.archetypeId.IsValid());
            auto it = archetypesMap.find(record.archetypeId);
            ASSERT(it != archetypesMap.end());
            archetype = &(*it->second);
            index = record.index;
            return true;
        }

        template <typename EventType>
        void Emit(EventType&& event)
        {
            static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
            eventStorage.Push({}, std::forward<EventType>(event));
        }

        template <typename EventType>
        void Emit(Ecs::Entity entity, EventType&& event)
        {
            Emit<EventType>(entity.GetId(), std::forward<EventType>(event));
        }

        template <typename EventType>
        void Emit(EntityId entity,EventType&& event)
        {
            ASSERT(entity);
            static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
            eventStorage.Push(entity, std::forward<EventType>(event));
        }

        template <typename EventType>
        void EmitImmediately(const EventType& event) const
        {
            static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
            broadcastEventImmediately(event);
        }

        template <typename EventType>
        void EmitImmediately(Ecs::Entity entity, const EventType& event) const
        {
            EmitImmediately<EventType>(entity.GetId(), event);
        }

        template <typename EventType>
        void EmitImmediately(EntityId entity, const EventType& event) const
        {
            ASSERT(entity);
            static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
            dispatchEventImmediately(entity, event);
        }

        void ProcessDefferedEvents();
        void Tick();

        World();

    private:
        template <typename U>
        friend struct EventBuilder;

        template <typename C, typename T>
        friend struct EntityBuilder;

        friend struct View;
        friend struct Query;
        friend struct QueryBuilder;
        friend struct SystemBuilder;

        static bool matches(const Archetype& archetype, const Ecs::View& view)
        {
            if LIKELY (!archetype.HasAll(SortedComponentsView(view.require)) ||
                       archetype.HasAny(SortedComponentsView(view.exclude)))
                return false;

            return true;
        }

        void updateCache(SystemId id)
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

        void updateCache(Archetype& archetype)
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

        QueryId _register(const Ecs::View& view);

        template <typename Components, typename ArgsTuple>
        EntityId commit(EntityId entity, SortedComponentsView removeComponents, ArgsTuple&& args)
        {
            return commitImpl<Components>(entity, removeComponents, eastl::forward<ArgsTuple>(args), eastl::make_index_sequence<Components::Count>());
        }

        ComponentInfo getComponentInfo(ComponentId id)
        {
            ASSERT(componentStorage.find(id) != componentStorage.end());
            return componentStorage[id];
        }

        Archetype& getOrCreateArchetype(ArchetypeId archetypeId, SortedComponentsView components)
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

                updateCache(*archetype);
            }
            else
                archetype = it->second.get();

            return *archetype;
        }

        template <typename Component, typename ArgsTuple, size_t... Index>
        void constructComponent(Archetype& archetype, ArchetypeEntityIndex index, ArgsTuple&& args, eastl::index_sequence<Index...>)
        {
            if constexpr (IsTag<Component>)
                return;

            auto* ptr = archetype.GetData(GetComponentId<Component>, index);
            new (ptr) Component {std::forward<decltype(std::get<Index>(args))>(std::get<Index>(args))...};
        }

        template <typename Components, typename ArgsTuple, size_t... Index>
        EntityId commitImpl(EntityId entity, SortedComponentsView removeComponents, ArgsTuple&& args, eastl::index_sequence<Index...>)
        {
            ComponentsSet components;
            Archetype* from = nullptr;
            ArchetypeEntityIndex fromIndex;

            if (entity)
            {
                if (!IsAlive(entity))
                {
                    ASSERT(false);
                    return entity;
                }

                if (!ResolveEntityArhetype(entity, from, fromIndex))
                {
                    ASSERT(false); // Impossible
                    return entity;
                }

                for (auto component : from->GetComponentsView())
                    components.push_back_unsorted(component); // Components already sorted
            }
            else
            {
                entity = entityStorage.Create();
                components.push_back_unsorted(GetComponentId<EntityId>);
            }

            ComponentsSet added;
            (RegisterComponent<typename Components::template Get<Index>>(), ...);
            auto addComponent = [&components](ComponentId id) -> int {
                bool added = components.insert(id).second;
                ASSERT_MSG(added, "Only new components can be added");
                return 0;
            };

            (addComponent(GetComponentId<typename Components::template Get<Index>>), ...);
            for (auto component : removeComponents)
                components.erase(component);

            ArchetypeId archetypeId = GetArchetypeIdForComponents(SortedComponentsView(components));
            Archetype& to = getOrCreateArchetype(archetypeId, SortedComponentsView(components));
            if (from == &to)
                return entity;

            ArchetypeEntityIndex index = from ? to.Mutate(entityStorage, *from, fromIndex) : to.Insert(entityStorage, entity);

            // Component data initialization
            (
                constructComponent<typename Components::template Get<Index>>(
                    to, index,
                    eastl::move(std::get<Index>(args)),
                    eastl::make_index_sequence<std::tuple_size_v<std::decay_t<decltype(std::get<Index>(args))>>>()),
                ...);

            UNUSED(index);
            // TODO validate remove components and add/remove at some thime.

            return entity;
        }

        template <typename Callable>
        void query(MatchedArchetypeSpan span, Callable&& callable, const Ecs::Event* event)
        {
            IterationContext context {*this, event};

            // Todo check all args in callable persist in requireComps with std::includes
            for (auto archetype : span)
                ArchetypeIterator::ForEach(*archetype, eastl::forward<Callable>(callable), context);
        }

        template <typename Callable>
        void query(const Ecs::Query& query, Callable&& callable)
        {
            // Todo check all args in callable persist in requireComps with std::includes
            MatchedArchetypeCache* archetypes = nullptr;
            cacheForQueriesView.ForEntity(EntityId(query.id.GetRaw()), [&archetypes](MatchedArchetypeCache& cache) {
                archetypes = &cache;
            });

            ASSERT(archetypes);
            this->query(MatchedArchetypeSpan(*archetypes), eastl::forward<Callable>(callable), nullptr);
        }
/*
        template <typename Callable>
        void queryForEntity(EntityId entityId, const Ecs::Query& query, Callable&& callable)
        {
            View* view = nullptr;
            /// Todo. i have hadache don't sure it's optimal
            cacheForQueriesView.ForEntity(EntityId(query.id.GetRaw()), [this](View& v) {
                view = &v;
            });
            ASSE(view);

            queryForEntity(entityId, *view, eastl::forward<Callable>(callable));
        }*/

        template <typename Callable>
        void query(const Ecs::View& view, Callable&& callable)
        {
            // Todo check all args in callable persist in requireComps with std::includes
            IterationContext context {*this, nullptr};

            for (auto it = archetypesMap.begin(); it != archetypesMap.end(); it++)
            {
                const Archetype& archetype = *it->second;
                if (!matches(archetype, view))
                    continue;

                ArchetypeIterator::ForEach(archetype, eastl::forward<Callable>(callable), context);
            }
        }

        template <typename Callable>
        void queryForEntity(const Ecs::Event& event, EntityId entityId, Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            queryForEntityImpl<ArgList>(entityId, eastl::forward<Callable>(callable), {*this, &event});
        }

        template <typename Callable>
        void queryForEntity(EntityId entityId, const Ecs::View& view, Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            queryForEntityImpl<ArgList>(entityId, view, eastl::forward<Callable>(callable), {*this, nullptr});
        }

        template <typename ArgumentList, typename Callable>
        void queryForEntityImpl(EntityId entityId, const Ecs::View& view, Callable&& callable, const IterationContext& context)
        {
            ASSERT(entityId);
            // Todo check all args in callable persist in requireComps with std::includes
            // Todo check entity are ok for requireComps and Args

            Archetype* archetype = nullptr;
            ArchetypeEntityIndex index;

            if (!ResolveEntityArhetype(entityId, archetype, index))
            {
                ASSERT(false);
                return;
            }

            if (!matches(*archetype, view))
            {
                ASSERT(false);
                return;
            }

            ArchetypeIterator::ForEntity(*archetype, index, eastl::forward<Callable>(callable), context);
        }

        // TODO fix this mess
        template <typename ArgumentList, typename Callable>
        void queryForEntityImpl(EntityId entityId, Callable&& callable, const IterationContext& context)
        {
            ASSERT(entityId);
            // Todo check all args in callable persist in requireComps with std::includes
            // Todo check entity are ok for requireComps and Args

            Archetype* archetype = nullptr;
            ArchetypeEntityIndex index;

            if (!ResolveEntityArhetype(entityId, archetype, index))
            {
                ASSERT(false);
                return;
            }

            ArchetypeIterator::ForEntity(*archetype, index, eastl::forward<Callable>(callable), context);
        }


        template <typename... Components>
        static constexpr ArchetypeId getArhetypeIdForComponents(TypeList<Components...>)
        {
            return ArchetypeInfo<Components...>::Id;
        }

        template <typename... Components>
        static constexpr auto getComponentsInfoArray(TypeList<Components...>)
        {
            return eastl::array<ComponentInfo, TypeList<Components...>::Count> {GetComponentInfo<Components>...};
        }

        void broadcastEventImmediately(const Ecs::Event& event) const;
        void dispatchEventImmediately(EntityId entity, const Ecs::Event& event) const;

    private:
        EntityStorage entityStorage;
        EventStorage eventStorage;
        // eastl::vector<QueryData> views;
        eastl::vector<SystemDescription> systems;
        // SystemStorage systemStorage;
        ComponentStorage componentStorage;
        Ecs::View cacheForQueriesView;
        Ecs::View cacheForSystemsView;
        Ecs::QueryId cacheForQueriesQuery;
        ska::flat_hash_map<EventId, eastl::fixed_vector<SystemId, 16>> eventsToSystems; // Todo rename
        ska::flat_hash_map<ArchetypeId, eastl::unique_ptr<Archetype>> archetypesMap;
    };

    template <typename Callable>
    void View::ForEach(Callable&& callable) const
    {
        world.query(*this, eastl::forward<Callable>(callable));
    }

    // TODO Maybe move it wold, if we have entity, we don't need view!
    template <typename Callable>
    void View::ForEntity(EntityId entityId, Callable&& callable) const
    {
        world.queryForEntity(entityId, *this, eastl::forward<Callable>(callable));
    }

    template <typename Callable>
    void View::ForEntity(Entity entity, Callable&& callable) const
    {
        ForEntity(entity.GetId(), eastl::forward<Callable>(callable));
    }

    template <typename Callable>
    void Query::ForEach(Callable&& callable) const
    {
        world.query(*this, eastl::forward<Callable>(callable));
    }

    inline Query QueryBuilder::Build() &&
    {
        auto& world = view.world;
        return Query(world, world._register(eastl::move(view)));
    }
}