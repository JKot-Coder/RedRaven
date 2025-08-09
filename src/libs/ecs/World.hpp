#pragma once

#include "common/NonCopyableMovable.hpp"
#include "ecs/ForwardDeclarations.hpp"

#include "ecs/Archetype.hpp"
#include "ecs/CommandBuffer.hpp"
#include "ecs/ComponentStorage.hpp"
#include "ecs/meta/ComponentTraits.hpp"
#include "ecs/Entity.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/EntityStorage.hpp"
#include "ecs/Event.hpp"
#include "ecs/EventStorage.hpp"
#include "ecs/Hash.hpp"
#include "ecs/IterationHelpers.hpp"
#include "ecs/Query.hpp"
#include "ecs/System.hpp"
#include "ecs/View.hpp"

#include "absl/container/flat_hash_map.h" // IWYU pragma: export
#include "absl/container/flat_hash_set.h" // IWYU pragma: export

#include <thread>

#ifdef ENABLE_ASSERTS
#define ASSERT_IS_CREATION_THREAD ASSERT(creationThreadID == std::this_thread::get_id())
#else
#define ASSERT_IS_CREATION_THREAD
#endif

namespace RR::Ecs
{
#ifdef ENABLE_ASSERTS
    namespace Debug
    {
        template <typename ComponentType>
        bool IsComponentInView(const Ecs::View& view)
        {
            static_assert(IsComponent<ComponentType>, "ComponentType must be a component type");

            // Todo replace with methods to make it more clear.
            if constexpr (eastl::is_same_v<ComponentType, Ecs::World> ||
                          eastl::is_base_of_v<Ecs::Event, ComponentType> ||
                          eastl::is_same_v<ComponentType, Ecs::EntityId>)
            {
                return true;
            }
            else
            {
                constexpr ComponentId compId = GetComponentId<ComponentType>;
                return view.GetWithSet().find(compId) != view.GetWithSet().end();
            }
        }

        template <typename Arg>
        void ValidateArgumentAgainstView(const Ecs::View& view)
        {
            if constexpr (!eastl::is_pointer_v<Arg>)
            {
                using ComponentType = GetComponentType<Arg>;
                ASSERT_MSG(
                    IsComponentInView<ComponentType>(view),
                    "Component {} used in lambda is not specified in the View's require set. Check component type and View definition.",
                    GetTypeName<ComponentType>);
            }
        }

        template<typename ArgumentList, size_t... Indices>
        void ValidateArgumentsAgainstView(const Ecs::View& view, eastl::index_sequence<Indices...>)
        {
            (ValidateArgumentAgainstView<typename ArgumentList::template Get<Indices>>(view), ...);
        }

        template<typename Callable>
        void ValidateLambdaArgumentsAgainstView(const Ecs::View& view, Callable&& /*callable*/)
        {
            using ArgList = GetArgumentList<Callable>;

            if constexpr (ArgList::Count > 0)
                ValidateArgumentsAgainstView<ArgList>(view, eastl::make_index_sequence<ArgList::Count>());
        }
    }
#endif

    struct World
    {
    private:
        using MatchedArchetypeCache = eastl::fixed_vector<const Archetype*, 16>;

        struct LockGuard : public Common::NonCopyableMovable
        {
            explicit LockGuard(World* world) : world(world) { world->lock(); }
            ~LockGuard() { world->unlock(); }

        private:
            World* world;
        };

    public:
        World();

        [[nodiscard]] bool ResolveEntityRecord(EntityId entityId, EntityRecord& record) const
        {
            ASSERT_IS_CREATION_THREAD;
            return entityStorage.Get(entityId, record);
        }

        [[nodiscard]] bool IsAlive(EntityId entityId) const;
        [[nodiscard]] bool Has(EntityId entityId, SortedComponentsView components) const;
        void Destroy(EntityId entityId);

        [[nodiscard]] Ecs::EntityBuilder<void, void> Entity();
        [[nodiscard]] Ecs::Entity EmptyEntity();
        [[nodiscard]] Ecs::Entity GetEntity(EntityId entityId) { return Ecs::Entity(*this, entityId); }

        [[nodiscard]] Ecs::View View() { return Ecs::View(*this); }
        [[nodiscard]] Ecs::QueryBuilder Query() { return Ecs::QueryBuilder(*this); }
        [[nodiscard]] Ecs::Query GetQuery(QueryId queryId) { return Ecs::Query(*this, queryId); }

        [[nodiscard]] Ecs::SystemBuilder System();
        [[nodiscard]] Ecs::SystemBuilder System(const HashName& name);
        [[nodiscard]] Ecs::System GetSystem(SystemId systemId) { return Ecs::System(*this, systemId); }

        template <typename Component>
        ComponentId RegisterComponent() { return componentStorage.Register<Component>(); }

        template <typename EventType>
        void Emit(EventType&& event);
        template <typename EventType>
        void Emit(Ecs::Entity entity, EventType&& event);
        template <typename EventType>
        void Emit(EntityId entity, EventType&& event);
        template <typename EventType>
        void EmitImmediately(const EventType& event) const;
        template <typename EventType>
        void EmitImmediately(Ecs::Entity entity, const EventType& event) const;
        template <typename EventType>
        void EmitImmediately(EntityId entity, const EventType& event) const;

        void RunSystem(SystemId systemId) const;
        void OrderSystems();
        void ProcessDefferedEvents();
        void ProcessTrackedChanges();
        void Tick();

        [[nodiscard]] bool IsLocked() const noexcept { return lockCounter > 0u; }

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
        friend struct Archetype;

    private:
        void destroyImpl(EntityId entityId);

        Ecs::System createSystem(SystemDescription&& desc, Ecs::View&& view, HashName&& name);
        Ecs::Query createQuery(Ecs::View&& view);

        void initCache(SystemId id);
        void initCache(QueryId id);
        void initCache(Archetype& archetype);

        static bool matches(const Archetype& archetype, const Ecs::View& view)
        {
            return !(!archetype.HasAll(SortedComponentsView(view.with)) ||
                    archetype.HasAny(SortedComponentsView(view.without)));
        }

        Archetype& createArchetypeNoCache(ArchetypeId archetypeId, SortedComponentsView components);
        Archetype& getOrCreateArchetype(ArchetypeId archetypeId, SortedComponentsView components);

        void handleDisappearEvent(EntityId entity, const Archetype& from, const Archetype& to);
        void handleAppearEvent(EntityId entity, const Archetype* from, const Archetype& to);
        // Dispatch event to systems, that are subscribed to this event.
        // Systems would be queried for all matched entities.
        void broadcastEventImmediately(const Ecs::Event& event) const;
        // Dispatch event to specific system and all entities in span.
        // System would be called once for each entity in span with specific event.
        void dispatchEventImmediately(ArchetypeEntitySpan span, SystemId systemId, const Ecs::Event& event) const;
        // Dispatch event to specific system and antity.
        // System would be called once for specific entity with specific event.
        void dispatchEventImmediately(EntityId entityId, SystemId systemId, const Ecs::Event& event) const;
        // Dispatch event to systems, that are subscribed to this event.
        // Systems would be queried for specific entity.
        void unicastEventImmediately(EntityId entity, const Ecs::Event& event) const;

        template <typename Component, typename ArgsTuple>
        void constructComponent(Archetype& archetype, ArchetypeEntityIndex index, ArgsTuple&& args);
        template <typename Callable>
        void mutateEntity(EntityId entityId, Archetype* from, ArchetypeEntityIndex fromIndex, Archetype& to, Callable&& constructComponents);
        template <typename Components, typename ArgsTuple, size_t... Index>
        [[nodiscard]] EntityId commit(EntityId entityId, SortedComponentsView removeComponents, ArgsTuple&& args, eastl::index_sequence<Index...> indexSeq);

        template <typename Callable>
        void invokeForEntities(ArchetypeEntitySpan span, const Ecs::Event* event, Callable&& callable);

        template <typename Callable>
        void query(QueryId queryId, Callable&& callable);
        template <typename Callable>
        void query(const Ecs::View& view, Callable&& callable);
        template <typename Callable>
        void queryForEntity(EntityId entityId, const Ecs::View& view, Callable&& callable);

        void lock() noexcept { ++lockCounter; }
        void unlock() noexcept
        {
            if (lockCounter > 0u)
                --lockCounter;
            if (lockCounter == 0u)
                onUnlock();
        }
        void onUnlock() { commandBuffer.ProcessCommands(*this); }

    private:
        bool systemsOrderDirty = false;
        uint32_t lockCounter {0u};
        std::thread::id creationThreadID;
        EntityStorage entityStorage;
        EventStorage eventStorage;
        ComponentStorage componentStorage;
        CommandBuffer commandBuffer;
        Ecs::View queriesView;
        Ecs::View systemsView;
        Ecs::QueryId queriesQuery;
        Ecs::QueryId systemsQuery;
        absl::flat_hash_set<ComponentId, Ecs::DummyHasher<ComponentId>> singletonsSet;
        absl::flat_hash_map<EventId, eastl::fixed_vector<SystemId, 16>, Ecs::DummyHasher<EventId>> eventSubscribers;
        absl::flat_hash_map<ArchetypeId, eastl::unique_ptr<Archetype>, Ecs::DummyHasher<ArchetypeId>> archetypesMap;
        eastl::vector<Archetype*> archetypesCache;
    };

    inline bool World::IsAlive(EntityId entityId) const
    {
        ASSERT_IS_CREATION_THREAD;

        EntityRecord record;
        if (!ResolveEntityRecord(entityId, record))
            return false;

        return record.IsAlive(IsLocked());
    }

    inline bool World::Has(EntityId entityId, SortedComponentsView components) const
    {
        ASSERT_IS_CREATION_THREAD;
        EntityRecord record;

        if (!ResolveEntityRecord(entityId, record))
            return false;

        Archetype* archetype = record.GetArchetype(IsLocked());
        return (archetype != nullptr) ? archetype->HasAll(components) : false;
    }

    template <typename Component, typename ArgsTuple>
    inline void World::constructComponent(Archetype& archetype, ArchetypeEntityIndex index, ArgsTuple&& args)
    {
        if constexpr (!IsTag<Component>)
        {
            std::apply(
                [&archetype, index](auto&&... unpackedArgs) {
                    const auto componentIndex = archetype.GetComponentIndex(GetComponentId<Component>);
                    archetype.ConstructComponent<Component>(index, componentIndex, eastl::forward<decltype(unpackedArgs)>(unpackedArgs)...);
                },
                eastl::forward<ArgsTuple>(args));
        }
    }

    template <typename Callable>
    inline void World::mutateEntity(EntityId entityId, Archetype* from, ArchetypeEntityIndex fromIndex, Archetype& to, Callable&& constructComponents)
    {
        ASSERT_IS_CREATION_THREAD;
        ASSERT(entityId);

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
    }

    template <typename Callable>
    inline void World::invokeForEntities(ArchetypeEntitySpan span, const Ecs::Event* event, Callable&& callable)
    {
        ASSERT_IS_CREATION_THREAD;
        IterationContext context {*this, event};

        LockGuard lg(this);
        // Todo check all args in callable persist in archetype.
        ArchetypeIterator::ForEach(span, context, eastl::forward<Callable>(callable));
    }

    template <typename Callable>
    inline void World::query(QueryId queryId, Callable&& callable)
    {
        ASSERT_IS_CREATION_THREAD;

        MatchedArchetypeCache* archetypes = nullptr;
        const Ecs::View* queryView = nullptr;
        queriesView.ForEntity(EntityId(queryId.GetRaw()), [&archetypes, &queryView](MatchedArchetypeCache& cache, const Ecs::View& view) {
            archetypes = &cache;
            queryView = &view;
        });

        ASSERT(archetypes);
        ASSERT(queryView);

        #ifdef ENABLE_ASSERTS
            Debug::ValidateLambdaArgumentsAgainstView(*queryView, callable);
        #endif
        for (auto archetype : *archetypes)
        {
            ArchetypeEntitySpan span(*archetype, archetype->begin(), archetype->end());
            invokeForEntities(span, nullptr, eastl::forward<Callable>(callable));
        }
    }

    template <typename Callable>
    inline void World::query(const Ecs::View& view, Callable&& callable)
    {
        ASSERT_IS_CREATION_THREAD;
        LockGuard lg(this);

        #ifdef ENABLE_ASSERTS
            Debug::ValidateLambdaArgumentsAgainstView(view, callable);
        #endif

        IterationContext context {*this, nullptr};

        for (const auto* archetype : archetypesCache)
        {
            if LIKELY (!matches(*archetype, view))
                continue;

            const ArchetypeEntitySpan span(*archetype, archetype->begin(), archetype->end());
            ArchetypeIterator::ForEach(span, context, eastl::forward<Callable>(callable));
        }
    }

    template <typename Callable>
    inline void World::queryForEntity(EntityId entityId, const Ecs::View& view, Callable&& callable)
    {
        ASSERT_IS_CREATION_THREAD;
        ASSERT(entityId);

        #ifdef ENABLE_ASSERTS
            Debug::ValidateLambdaArgumentsAgainstView(view, callable);
        #endif

        EntityRecord record;
        if (!ResolveEntityRecord(entityId, record))
        {
            ECS_VERIFY(false, "Deleted or non existing entity.");
            return;
        }

        if (record.HasPendingChanges())
        {
            ECS_VERIFY(false, "Can't query entity with pending changes.");
            return;
        }

        Archetype *archetype = record.GetArchetype(IsLocked());
        if UNLIKELY (!matches(*archetype, view))
        {
            ECS_VERIFY(false, "View doesn't match with entity.");
            return;
        }

        LockGuard lg(this);
        ArchetypeEntityIndex index = record.GetIndex(false);
        ArchetypeIterator::ForEntity(*archetype, index, {*this, nullptr}, eastl::forward<Callable>(callable));
    }

    template <typename Components, typename ArgsTuple, size_t... Index>
    EntityId World::commit(EntityId entityId, SortedComponentsView removeComponents, ArgsTuple&& args, eastl::index_sequence<Index...> indexSeq)
    {
        Archetype* from = nullptr;
        ArchetypeEntityIndex fromIndex;

        if (entityId)
        {
            if (!IsAlive(entityId))
                return entityId;

            EntityRecord record;
            if (!ResolveEntityRecord(entityId, record))
            {
                ASSERT(false); // Impossible
                return entityId;
            }

            from = record.GetArchetype(IsLocked());
            if (!IsLocked())
                fromIndex = record.GetIndex(false);
        }


        (RegisterComponent<typename Components::template Get<Index>>(), ...);

        auto getComponentName = [this](ComponentId id) -> std::string {
            auto it = componentStorage.find(id);
            if (it == componentStorage.end())
                return fmt::format("<{:#X}>", id.GetRaw());
            return std::string(it->second.name);
        };

        bool onlyNewSingletons = ([&getComponentName, this]() -> bool {
            UNUSED(this, getComponentName);

            using T = typename Components::template Get<Index>;
            if constexpr (IsSingleton<T>)
            {
                auto id = GetComponentId<T>;
                if (singletonsSet.find(id) != singletonsSet.end())
                {
                    ECS_VERIFY(false, "Singleton component {} is already exists. Commit will be ignored.", getComponentName(id));
                    return false;
                }

                singletonsSet.insert(id);
                return true;
            }
            else
            {
                return true;
            }
        }() && ...);

        if (!onlyNewSingletons)
            return entityId;

        ComponentsSet components;
        ComponentsSet added;

        if (from)
        {
            for (auto component : from->GetComponentsView())
                components.push_back_unsorted(component); // Components already sorted

            for (auto component : removeComponents)
            {
                [[maybe_unused]] auto result = components.erase(component);

                // We could silent this error, if it's would be a case reconsider this.
                ECS_VERIFY(result == 1, "Can't remove component {}. Component is not present in the archetype.", getComponentName(component));
            }
        }
        else
        {
            components.push_back_unsorted(GetComponentId<EntityId>); // Adding first component.
            ECS_VERIFY(eastl::distance(removeComponents.begin(), removeComponents.end()) == 0, "Can't remove components on creation of entity.");
        }

        auto addComponent = [&components, &getComponentName](ComponentId id) -> int {
            [[maybe_unused]] bool added = components.insert(id).second;
            UNUSED(getComponentName);
            // We could silent this error, if it's would be a case reconsider this.
            ECS_VERIFY(added, "Can't add component {}. Only new components can be added.", getComponentName(id));
            return 0;
        };

        eastl::array<ComponentId, Components::Count> addedComponents = {GetComponentId<typename Components::template Get<Index>>...};
        for (auto component : addedComponents)
            addComponent(component);

        ArchetypeId archetypeId = GetArchetypeIdForComponents(SortedComponentsView(components));
        Archetype& to = getOrCreateArchetype(archetypeId, SortedComponentsView(components));

#ifdef ECS_ENABLE_CHEKS
        eastl::quick_sort(addedComponents.begin(), addedComponents.end());
        ECS_VERIFY(!SortedComponentsView(addedComponents).IsIntersects(removeComponents), "Can't add and remove components at the same time.");
#endif

        if (!IsLocked())
        {
            if (!entityId)
                entityId = entityStorage.Create(to);
            mutateEntity(entityId, from, fromIndex, to, [&](Archetype& archetype, ArchetypeEntityIndex index) {
                (
                    constructComponent<typename Components::template Get<Index>>(
                        archetype, index,
                        eastl::forward<std::tuple_element_t<Index, ArgsTuple>>(std::get<Index>(args))),
                    ...);
            });
        }
        else
        {
            if (!entityId)
                entityId = entityStorage.CreateAsync(to);
            else
                entityStorage.PendingMutate(entityId, to);
            commandBuffer.Mutate<Components>(entityId, from, to, eastl::forward<ArgsTuple>(args), indexSeq);
        }

        ASSERT(entityId);
        return entityId;
    }

    template <typename EventType>
    inline void World::Emit(EventType&& event)
    {
        static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
        ASSERT_IS_CREATION_THREAD;
        eventStorage.Push({}, std::forward<EventType>(event));
    }

    template <typename EventType>
    inline void World::Emit(Ecs::Entity entity, EventType&& event)
    {
        ASSERT(entity.world == this);
        Emit<EventType>(entity.GetId(), std::forward<EventType>(event));
    }

    template <typename EventType>
    inline void World::Emit(EntityId entity, EventType&& event)
    {
        static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
        ASSERT_IS_CREATION_THREAD;
        ASSERT(entity);
        eventStorage.Push(entity, std::forward<EventType>(event));
    }

    template <typename EventType>
    inline void World::EmitImmediately(const EventType& event) const
    {
        static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
        broadcastEventImmediately(static_cast<const Ecs::Event&>(event));
    }

    template <typename EventType>
    inline void World::EmitImmediately(Ecs::Entity entity, const EventType& event) const
    {
        ASSERT(entity.world == this);
        EmitImmediately<EventType>(entity.GetId(), event);
    }

    template <typename EventType>
    inline void World::EmitImmediately(EntityId entity, const EventType& event) const
    {
        ASSERT(entity);
        static_assert(eastl::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
        unicastEventImmediately(entity, event);
    }

    inline void Entity::Destroy() const { world->Destroy(id); }
    inline bool Entity::IsAlive() const { return world->IsAlive(id); }
    inline bool Entity::Has(SortedComponentsView componentsView) const { return world->Has(id, componentsView); }
    inline bool Entity::ResolveArhetype(Archetype*& archetype, ArchetypeEntityIndex& index) const
    {
        EntityRecord record;
        if (!world->ResolveEntityRecord(id, record))
            return false;

        // TODO. Any pending mutations/deletion will be ignored. Add message/asset about it?
        archetype = record.GetArchetype(false);
        index = record.GetIndex(false);
        return true;
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