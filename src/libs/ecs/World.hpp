#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/EntityStorage.hpp"
#include "ecs/ComponentStorage.hpp"
#include "ecs/Query.hpp"
#include "ecs/View.hpp"
#include "ecs/System.hpp"
#include "ecs/IterationHelpers.hpp"
#include "ecs/Entity.hpp"
#include "ecs/EntityId.hpp"
#include "absl/container/flat_hash_map.h"

namespace RR::Ecs
{
    struct World
    {
    private:
        using MatchedArchetypeCache = eastl::fixed_vector<const Archetype*, 16>; // TODO naming?

    public:
        [[nodiscard]] Ecs::SystemBuilder System();
        [[nodiscard]] Ecs::SystemBuilder System(const HashName& name);
        [[nodiscard]] Ecs::System GetSystem(SystemId systemId) { return Ecs::System(*this, systemId); }
        [[nodiscard]] Ecs::EntityBuilder<void, void> Entity();
        [[nodiscard]] Ecs::Entity EmptyEntity();
        [[nodiscard]] Ecs::Entity GetEntity(EntityId entityId) { return Ecs::Entity(*this, entityId); }
        [[nodiscard]] Ecs::View View() { return Ecs::View(*this); }
        [[nodiscard]] Ecs::QueryBuilder Query() { return Ecs::QueryBuilder(*this); }
        [[nodiscard]] Ecs::Query GetQuery(QueryId queryId) { return Ecs::Query(*this, queryId); }

        [[nodiscard]] bool IsAlive(EntityId entityId) const { return entityStorage.IsAlive(entityId); }

        [[nodiscard]] bool Has(EntityId entityId, SortedComponentsView components)
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
            {
                unicastEventImmediately(entityId, OnDissapear {});
                archetype->Delete(entityStorage, index, false);
            }
            entityStorage.Destroy(entityId);
        }

        template <typename Component>
        ComponentId RegisterComponent() { return componentStorage.Register<Component>(); }

        [[nodiscard]] bool ResolveEntityArhetype(EntityId entity, Archetype*& archetype, ArchetypeEntityIndex& index) const
        {
            EntityRecord record;
            if (!entityStorage.Get(entity, record))
                return false;

            archetype = record.archetype;
            index = record.index;

            ASSERT(archetype);
            ASSERT(index);

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
        void Emit(EntityId entity, EventType&& event)
        {
            ASSERT(entity);
            static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
            eventStorage.Push(entity, std::forward<EventType>(event));
        }

        template <typename EventType>
        void EmitImmediately(const EventType& event) const
        {
            static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
            broadcastEventImmediately(static_cast<const Ecs::Event&>(event));
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
            unicastEventImmediately(entity, event);
        }

        void RunSystem(SystemId systemId) const;
        void OrderSystems();
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

        Ecs::System createSystem(SystemDescription&& desc, Ecs::View&& view, HashName&& name);
        Ecs::Query createQuery(Ecs::View&& view);

        static bool matches(const Archetype& archetype, const Ecs::View& view)
        {
            if LIKELY (!archetype.HasAll(SortedComponentsView(view.require)) ||
                       archetype.HasAny(SortedComponentsView(view.exclude)))
                return false;

            return true;
        }

        void initCache(SystemId id);
        void initCache(QueryId id);
        void initCache(Archetype& archetype);

        template <typename Components, typename ArgsTuple>
        EntityId commit(EntityId entity, SortedComponentsView removeComponents, ArgsTuple&& args)
        {
            return commitImpl<Components>(entity, removeComponents, eastl::forward<ArgsTuple>(args), eastl::make_index_sequence<Components::Count>());
        }

        // Private, do not use this directly.
        Archetype& createArchetypeNoCache(ArchetypeId archetypeId, SortedComponentsView components);
        Archetype& getOrCreateArchetype(ArchetypeId archetypeId, SortedComponentsView components);

        template <typename Component, typename ArgsTuple, size_t... Index>
        void constructComponent(Archetype& archetype, ArchetypeEntityIndex index, ArgsTuple&& args, eastl::index_sequence<Index...>)
        {
            if constexpr (IsTag<Component>)
                return;

            // TODO this could be reused from mutate.
            const auto componentIndex = archetype.GetComponentIndex<Component>();
            auto* ptr = archetype.GetComponentData(componentIndex, index);
            new (ptr) Component {eastl::forward<decltype(std::get<Index>(args))>(std::get<Index>(args))...};
        }

        template <typename Components, typename ArgsTuple, size_t... Index>
        EntityId commitImpl(EntityId entityId, SortedComponentsView removeComponents, ArgsTuple&& args, eastl::index_sequence<Index...>)
        {
            ComponentsSet components;
            Archetype* from = nullptr;
            ArchetypeEntityIndex fromIndex;

            if (entityId)
            {
                if (!IsAlive(entityId))
                    return entityId;

                if (!ResolveEntityArhetype(entityId, from, fromIndex))
                {
                    ASSERT(false); // Impossible
                    return entityId;
                }

                for (auto component : from->GetComponentsView())
                    components.push_back_unsorted(component); // Components already sorted

                for (auto component : removeComponents)
                    components.erase(component);
            }
            else
                ASSERT(eastl::distance(removeComponents.begin(), removeComponents.end()) == 0);

            ComponentsSet added;
            (RegisterComponent<typename Components::template Get<Index>>(), ...);
            auto addComponent = [&components](ComponentId id) -> int {
                bool added = components.insert(id).second;
                UNUSED(added);
                ASSERT_MSG(added, "Only new components can be added");
                return 0;
            };

            if (!entityId)
                components.push_back_unsorted(GetComponentId<EntityId>);

            (addComponent(GetComponentId<typename Components::template Get<Index>>), ...);

            ArchetypeId archetypeId = GetArchetypeIdForComponents(SortedComponentsView(components));
            Archetype& to = getOrCreateArchetype(archetypeId, SortedComponentsView(components));

            if (from && from == &to)
                return entityId;

            ArchetypeEntityIndex index;
            if (from)
            {
                handleDisappearEvent(entityId, *from, to);
                index = to.Mutate(entityStorage, *from, fromIndex);
            }
            else
            {
                index = to.Insert(entityStorage);
                entityId = to.GetEntityIdData(index);
            }

            // Component data initialization
            (
                constructComponent<typename Components::template Get<Index>>(
                    to, index,
                    eastl::move(std::get<Index>(args)),
                    eastl::make_index_sequence<std::tuple_size_v<std::decay_t<decltype(std::get<Index>(args))>>>()),
                ...);

            handleAppearEvent(entityId, from, to);

            ASSERT(entityId);
            // TODO validate remove components and add/remove at some thime.

            return entityId;
        }

        template <typename Callable>
        void query(MatchedArchetypeSpan span, const Ecs::Event* event, Callable&& callable)
        {
            IterationContext context {*this, event};

            // Todo check all args in callable persist in requireComps with std::includes
            for (auto archetype : span)
                ArchetypeIterator::ForEach(*archetype, context, eastl::forward<Callable>(callable));
        }

        template <typename Callable>
        void query(QueryId queryId, Callable&& callable)
        {
            // Todo check all args in callable persist in requireComps with std::includes
            MatchedArchetypeCache* archetypes = nullptr;
            queriesView.ForEntity(EntityId(queryId.GetRaw()), [&archetypes](MatchedArchetypeCache& cache) {
                archetypes = &cache;
            });

            ASSERT(archetypes);
            this->query(MatchedArchetypeSpan(*archetypes), nullptr, eastl::forward<Callable>(callable));
        }

        template <typename Callable>
        void query(const Ecs::View& view, Callable&& callable)
        {
            // Todo check all args in callable persist in requireComps with std::includes
            IterationContext context {*this, nullptr};

            for (const auto& archetype : archetypes)
            {
                if (!matches(*archetype, view))
                    continue;

                ArchetypeIterator::ForEach(*archetype, context, eastl::forward<Callable>(callable));
            }
        }

        template <typename Callable>
        void queryForEntity(EntityId entityId, Ecs::Event const* event, Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            queryForEntityImpl<ArgList>(entityId, {*this, event}, eastl::forward<Callable>(callable));
        }

        template <typename Callable>
        void queryForEntity(EntityId entityId, const Ecs::View& view, Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            queryForEntityImpl<ArgList>(entityId, view, {*this, nullptr}, eastl::forward<Callable>(callable));
        }

        template <typename ArgumentList, typename Callable>
        void queryForEntityImpl(EntityId entityId, const Ecs::View& view, const IterationContext& context, Callable&& callable)
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

            ArchetypeIterator::ForEntity(*archetype, index, context, eastl::forward<Callable>(callable));
        }

        // TODO fix this mess
        template <typename ArgumentList, typename Callable>
        void queryForEntityImpl(EntityId entityId, const IterationContext& context, Callable&& callable)
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

            ArchetypeIterator::ForEntity(*archetype, index, context, eastl::forward<Callable>(callable));
        }

        void handleDisappearEvent(EntityId entity, const Archetype& from, const Archetype& to);
        void handleAppearEvent(EntityId entity, const Archetype* from, const Archetype& to);
        void broadcastEventImmediately(const Ecs::Event& event) const;
        void dispatchEventImmediately(EntityId entity, SystemId systemId, const Ecs::Event& event) const;
        void unicastEventImmediately(EntityId entity, const Ecs::Event& event) const;

    private:
        bool systemsDirty = false;
        EntityStorage entityStorage;
        EventStorage eventStorage;
       // eastl::vector<SystemDescription> systems;
        // SystemStorage systemStorage;
        ComponentStorage componentStorage;
        Ecs::View queriesView;
        Ecs::View systemsView;
        Ecs::QueryId queriesQuery;
        Ecs::QueryId systemsQuery;
        absl::flat_hash_map<EventId, eastl::fixed_vector<SystemId, 16>> eventSubscribers; // Todo rename
        absl::flat_hash_map<ArchetypeId, ArchetypeIndex> archetypesMap;
        eastl::vector<eastl::unique_ptr<Archetype>> archetypes;
       // absl::flat_hash_map<EntityId, EntityRecord> entityRecords;
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
        world.query(id, eastl::forward<Callable>(callable));
    }

    inline Query QueryBuilder::Build() &&
    {
        return view.world.createQuery(eastl::move(view));
    }

    inline void System::Run() const
    {
        world->RunSystem(id);
    }
}