#pragma once

#include "ecs/Archetype.hpp"
#include "ecs/ComponentStorage.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/EntityStorage.hpp"
#include "ecs/Entity.hpp"
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

        using MatchedArchetypeCache = eastl::fixed_vector<const Archetype*, 16>; //TODO naming?

    public:
        template <typename EventType>
        inline EventBuilder<EventType> Event();
        SystemBuilder System();
        Ecs::Entity Entity();
        Ecs::Entity Entity(EntityId entityId);

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

        // TODO return void
        bool ResolveEntityArhetype(EntityId entity, Archetype*& archetype, ArchetypeEntityIndex& index)
        {
            EntityRecord record;
            if (!entityStorage.Get(entity, record))
                return false;

            // TODO this is should always valid in future
            if (record.archetypeId.IsValid())
            {
                auto it = archetypesMap.find(record.archetypeId);
                ASSERT(it != archetypesMap.end());
                if (it != archetypesMap.end())
                {
                    archetype = &(*it->second);
                    index = record.index;
                    return true;
                }
            }

            return false;
        }

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

        void updateCache(const Archetype& archetype)
        {
            // This could only happends while world creation process.
            // We create archetype to place this query, and while create archetype we tring use this query.
            if UNLIKELY(!cacheForQueriesQuery.IsValid())
                return;

            Ecs::Query(*this, cacheForQueriesQuery).ForEach([&archetype](Ecs::View& view, MatchedArchetypeCache& cache) {
                if (!matches(archetype, view))
                    return;

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

            size_t chunkSizePower = 7; // TODO move it somewhere

            auto it = archetypesMap.find(archetypeId);
            if (it == archetypesMap.end())
            {
                auto archUniqPtr = eastl::make_unique<Archetype>(
                    archetypeId,
                    chunkSizePower,
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

            auto* ptr = archetype.GetComponentData(GetComponentId<Component>)->GetData(index);
            new (ptr) Component {std::forward<decltype(std::get<Index>(args))>(std::get<Index>(args))...};
        }

        template <typename Components, typename ArgsTuple, size_t... Index>
        EntityId commitImpl(EntityId entity, SortedComponentsView removeComponents, ArgsTuple&& args, eastl::index_sequence<Index...>)
        {
            EntityRecord record;
            bool valid = entityStorage.Get(entity, record);
            ASSERT(valid);

            if (!valid)
                return entity;

            Archetype* from = nullptr;
            ArchetypeEntityIndex fromIndex;
            ComponentsSet components;
            ComponentsSet added;

            if (ResolveEntityArhetype(entity, from, fromIndex))
            {
                for (auto component : from->GetComponentsView())
                    components.push_back_unsorted(component); // Components already sorted
            }
            else
                components.push_back_unsorted(GetComponentId<EntityId>);

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
        void query(const Ecs::View& view, Callable&& callable)
        {
            // Todo check all args in callable persist in requireComps with std::includes
            for (auto it = archetypesMap.begin(); it != archetypesMap.end(); it++)
            {
                const Archetype& archetype = *it->second;
                if (!matches(archetype, view))
                    continue;

                ArchetypeIterator::ForEach(eastl::forward<Callable>(callable), {*this, archetype});
            }
        }

        template <typename Callable>
        void query(const Ecs::Query& query, Callable&& callable)
        {
            // Todo check all args in callable persist in requireComps with std::includes
            MatchedArchetypeCache* archetypes = nullptr;
            cacheForQueriesView.ForEntity(EntityId(query.id.Value()), [&archetypes](MatchedArchetypeCache& cache) {
                archetypes = &cache;
            });

            ASSERT(archetypes);
            for (auto archetype : *archetypes)
                ArchetypeIterator::ForEach(eastl::forward<Callable>(callable), {*this, *archetype});
        }

        template <typename Callable>
        void queryForEntity(EntityId entityId, const Ecs::View& view, Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            queryForEntityImpl<ArgList>(entityId, view, eastl::forward<Callable>(callable), eastl::make_index_sequence<ArgList::Count>());
        }

        template <typename ArgumentList, typename Callable, size_t... Index>
        void queryForEntityImpl(EntityId entityId, const Ecs::View& view, Callable&& callable, eastl::index_sequence<Index...>)
        {
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

            ArchetypeIterator::ForEntity(index, eastl::forward<Callable>(callable), {*this, *archetype});
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

        EntityId createEntity() { return entityStorage.Create(); }

        template <typename EventType>
        void emit(EventType&& event, const EventDescription& eventDesc);
        template <typename EventType>
        void emitImmediately(EventType&& event, const EventDescription& eventDesc) const;

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
        ska::flat_hash_map<TypeId, eastl::fixed_vector<SystemId, 8>> eventsToSystems; // Todo rename //TODO eventId
        ska::flat_hash_map<ArchetypeId, eastl::unique_ptr<Archetype>> archetypesMap;
    };

    template <typename EventType>
    void World::emit(EventType&& event, const EventDescription& eventDesc)
    {
        static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
        eventStorage.Push(eastl::forward<EventType>(event), eventDesc);
    }

    template <typename EventType>
    void World::emitImmediately(EventType&& event, const EventDescription& eventDesc) const
    {
        static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
        if (!eventDesc.entity)
            broadcastEventImmediately(eastl::forward<EventType>(event));
        else
            dispatchEventImmediately(eventDesc.entity, eastl::forward<EventType>(event));
    }

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