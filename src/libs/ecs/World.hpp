#pragma once

#include "ecs/ForwardDeclarations.hpp"

#include "ecs/Archetype.hpp"
#include "ecs/ComponentStorage.hpp"
#include "ecs/ComponentTraits.hpp"
#include "ecs/Entity.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/EntityStorage.hpp"
#include "ecs/Event.hpp"
#include "ecs/Hash.hpp"
#include "ecs/IterationHelpers.hpp"
#include "ecs/Query.hpp"
#include "ecs/System.hpp"
#include "ecs/View.hpp"

#include "absl/container/flat_hash_map.h" // IWYU pragma: export

#include <thread>

#ifdef ENABLE_ASSERTS
#define ASSERT_IS_CREATION_THREAD ASSERT(creationThreadID == std::this_thread::get_id())
#else
#define ASSERT_IS_CREATION_THREAD
#endif

namespace RR::Ecs
{
    struct World
    {
    private:
        using MatchedArchetypeCache = eastl::fixed_vector<const Archetype*, 16>;

        struct LockGuard
        {
            LockGuard(World* world) : world(world) { world->lock(); }
            ~LockGuard() { world->unlock(); }
            World* world;
        };

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

        [[nodiscard]] bool IsAlive(EntityId entityId) const
        {
            ASSERT_IS_CREATION_THREAD;
            return entityStorage.IsAlive(entityId);
        }

        [[nodiscard]] bool Has(EntityId entityId, SortedComponentsView components) const
        {
            ASSERT_IS_CREATION_THREAD;
            Archetype* archetype = nullptr;
            ArchetypeEntityIndex index;

            if (ResolveEntityArhetype(entityId, archetype, index))
                return archetype->HasAll(components);

            return false;
        }

        void Destruct(EntityId entityId)
        {
            ASSERT_IS_CREATION_THREAD;
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
            ASSERT_IS_CREATION_THREAD;
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
            ASSERT_IS_CREATION_THREAD;
            eventStorage.Push({}, std::forward<EventType>(event));
        }

        template <typename EventType>
        void Emit(Ecs::Entity entity, EventType&& event)
        {
            ASSERT(entity.world == this);
            Emit<EventType>(entity.GetId(), std::forward<EventType>(event));
        }

        template <typename EventType>
        void Emit(EntityId entity, EventType&& event)
        {
            static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
            ASSERT_IS_CREATION_THREAD;
            ASSERT(entity);
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
            ASSERT(entity.world == this);
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
        friend struct CommmandProcessors;
        friend struct LockGuard;

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

        template <typename Components, typename ArgsTuple, size_t... Index>
        EntityId commit(EntityId entityId, SortedComponentsView removeComponents, ArgsTuple&& args, eastl::index_sequence<Index...>)
        {
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
            }
            else
                entityId = entityStorage.Create();

            eastl::array<ComponentId, Components::Count> addedComponents = {GetComponentId<typename Components::template Get<Index>>...};
            (RegisterComponent<typename Components::template Get<Index>>(), ...);

            if (true)
            {
                mutateEntity(entityId, from, fromIndex, removeComponents, UnsortedComponentsView(addedComponents), [&](Archetype& archetype, ArchetypeEntityIndex index) {
                    (
                        constructComponent<typename Components::template Get<Index>>(
                            archetype, index,
                            eastl::forward<std::tuple_element_t<Index, ArgsTuple>>(std::get<Index>(args))),
                        ...);
                });
            }

            ASSERT(entityId);
            return entityId;
        }

        // Private, do not use this directly.
        Archetype& createArchetypeNoCache(ArchetypeId archetypeId, SortedComponentsView components);
        Archetype& getOrCreateArchetype(ArchetypeId archetypeId, SortedComponentsView components);

        template <typename Component, typename ArgsTuple>
        void constructComponent(Archetype& archetype, ArchetypeEntityIndex index, ArgsTuple&& args)
        {
            if constexpr (IsTag<Component>)
                return;

            std::apply(
                [&archetype, index](auto&&... unpackedArgs) {
                    // TODO this could be reused from mutate.
                    const auto componentIndex = archetype.GetComponentIndex(GetComponentId<Component>);
                    archetype.ConstructComponent<Component>(index, componentIndex, eastl::forward<decltype(unpackedArgs)>(unpackedArgs)...);
                },
                eastl::forward<ArgsTuple>(args));
        }

        template <typename Callable>
        void mutateEntity(EntityId entityId, Archetype* from, ArchetypeEntityIndex fromIndex, SortedComponentsView removeComponents, UnsortedComponentsView addedComponents, Callable&& constructComponents)
        {
            ASSERT_IS_CREATION_THREAD;
            ASSERT(entityId);

            ComponentsSet components;
            ComponentsSet added;

            if (from)
            {
                for (auto component : from->GetComponentsView())
                    components.push_back_unsorted(component); // Components already sorted

                for (auto component : removeComponents)
                    components.erase(component);
            }
            else
            {
                components.push_back_unsorted(GetComponentId<EntityId>);
                ASSERT(eastl::distance(removeComponents.begin(), removeComponents.end()) == 0);
            }

            auto addComponent = [&components](ComponentId id) -> int {
                bool added = components.insert(id).second;
                UNUSED(added);
                ASSERT_MSG(added, "Can't add component {}. Only new components can be added.", id.GetRaw());
                return 0;
            };

            for (auto component : addedComponents)
                addComponent(component);

            ArchetypeId archetypeId = GetArchetypeIdForComponents(SortedComponentsView(components));
            Archetype& to = getOrCreateArchetype(archetypeId, SortedComponentsView(components));

            if (from && from == &to)
                return;

            ArchetypeEntityIndex index;
            if (from)
            {
                handleDisappearEvent(entityId, *from, to);
                index = to.Mutate(entityStorage, *from, fromIndex);
            }
            else
            {
                index = to.Insert(entityId);
                entityStorage.Mutate(entityId, to, index);
            }
            constructComponents(to, index);

            handleAppearEvent(entityId, from, to);
            // TODO validate remove components and add/remove at some thime.
        }

        template <typename Callable>
        void query(MatchedArchetypeSpan span, const Ecs::Event* event, Callable&& callable)
        {
            ASSERT_IS_CREATION_THREAD;
            IterationContext context {*this, event};

            LockGuard lg(this);
            // Todo check all args in callable persist in requireComps with std::includes
            for (auto archetype : span)
                ArchetypeIterator::ForEach(*archetype, context, eastl::forward<Callable>(callable));
        }

        template <typename Callable>
        void query(QueryId queryId, Callable&& callable)
        {
            ASSERT_IS_CREATION_THREAD;
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
            ASSERT_IS_CREATION_THREAD;
            LockGuard lg(this);

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
            ASSERT_IS_CREATION_THREAD;
            using ArgList = GetArgumentList<Callable>;
            queryForEntityImpl<ArgList>(entityId, {*this, event}, eastl::forward<Callable>(callable));
        }

        template <typename Callable>
        void queryForEntity(EntityId entityId, const Ecs::View& view, Callable&& callable)
        {
            ASSERT_IS_CREATION_THREAD;
            using ArgList = GetArgumentList<Callable>;
            queryForEntityImpl<ArgList>(entityId, view, {*this, nullptr}, eastl::forward<Callable>(callable));
        }

        template <typename ArgumentList, typename Callable>
        void queryForEntityImpl(EntityId entityId, const Ecs::View& view, const IterationContext& context, Callable&& callable)
        {
            ASSERT_IS_CREATION_THREAD;
            ASSERT(entityId);

            LockGuard lg(this);
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
            ASSERT_IS_CREATION_THREAD;
            ASSERT(entityId);

            LockGuard lg(this);
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
        // Dispatch event to systems, that are subscribed to this event.
        // Systems would be queried for all matched entities.
        void broadcastEventImmediately(const Ecs::Event& event) const;
        // Dispatch event to specific system.
        // System would be queried for specific entity.
        // TODO not correct description.
        void dispatchEventImmediately(EntityId entity, SystemId systemId, const Ecs::Event& event) const;
        // Dispatch event to systems, that are subscribed to this event.
        // Systems would be queried for specific entity.
        void unicastEventImmediately(EntityId entity, const Ecs::Event& event) const;

        [[nodiscard]] bool isLocked() const noexcept { return lockCounter > 0u; }
        void lock() noexcept { ++lockCounter; }
        void unlock() noexcept
        {
            if (lockCounter > 0u)
                --lockCounter;
            if (lockCounter == 0u)
                onUnlock();
        }

        void onUnlock() {
        }

    private:
        bool systemsOrderDirty = false;
        uint32_t lockCounter{0u};
        std::thread::id creationThreadID;
        EntityStorage entityStorage;
        EventStorage eventStorage;
        ComponentStorage componentStorage;
        Ecs::View queriesView;
        Ecs::View systemsView;
        Ecs::QueryId queriesQuery;
        Ecs::QueryId systemsQuery;
        absl::flat_hash_map<EventId, eastl::fixed_vector<SystemId, 16>> eventSubscribers;
        absl::flat_hash_map<ArchetypeId, ArchetypeIndex> archetypesMap;
        eastl::vector<eastl::unique_ptr<Archetype>> archetypes;
    };

    inline void Entity::Destruct() const { world->Destruct(id); }
    inline bool Entity::IsAlive() const { return world->IsAlive(id); }
    inline bool Entity::Has(SortedComponentsView componentsView) const { return world->Has(id, componentsView); }
    inline bool Entity::ResolveArhetype(Archetype*& archetype, ArchetypeEntityIndex& index) const
    {
        return world->ResolveEntityArhetype(id, archetype, index);
    }

    template <typename Callable>
    inline void View::ForEach(Callable&& callable) const
    {
        world.query(*this, eastl::forward<Callable>(callable));
    }

    // TODO Maybe move it wold, if we have entity, we don't need view!
    template <typename Callable>
    inline void View::ForEntity(EntityId entityId, Callable&& callable) const
    {
        world.queryForEntity(entityId, *this, eastl::forward<Callable>(callable));
    }

    template <typename Callable>
    inline void View::ForEntity(Entity entity, Callable&& callable) const
    {
        ForEntity(entity.GetId(), eastl::forward<Callable>(callable));
    }

    template <typename Callable>
    inline void Query::ForEach(Callable&& callable) const
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