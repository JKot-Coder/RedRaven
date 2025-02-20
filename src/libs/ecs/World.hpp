#pragma once

#include "ecs/Archetype.hpp"
#include "ecs/ComponentStorage.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/EntityStorage.hpp"
#include "ecs/Event.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/FunctionTraits.hpp"
#include "ecs/IterationHelpers.hpp"
#include "ecs/QueryBuilder.hpp"
#include "ecs/System.hpp"
#include "ska/flat_hash_map.h"

namespace RR::Ecs
{
    struct World
    {
    public:
        template <typename EventType>
        inline EventBuilder<EventType> Event() const;
        template <typename... Components>
        inline SystemBuilder<Components...> System(const char* name);
        Ecs::Entity Entity();
        Ecs::Entity Entity(EntityId entityId);

        template <typename... Components>
        QueryBuilder Query() { return QueryBuilder(*this).Require<Components...>(); }

        template <typename T, typename DescriptionType>
        T Create(DescriptionType&& desc);

        bool IsAlive(EntityId entityId) const { return entityStorage.IsAlive(entityId); }

        // Components should be sorted
        template <typename ComponentIterator>
        bool Has(EntityId entityId, ComponentIterator begin, ComponentIterator end)
        {
            Archetype* archetype = nullptr;
            ArchetypeEntityIndex index;

            if (getArchetypeForEntity(entityId, archetype, index))
                return archetype->HasComponents(begin, end);

            return false;
        }

        void Destruct(EntityId entityId)
        {
            if (!IsAlive(entityId)) return;

            Archetype* archetype = nullptr;
            ArchetypeEntityIndex index;
            if (getArchetypeForEntity(entityId, archetype, index))
                archetype->Delete(entityStorage, index, false);

            entityStorage.Destroy(entityId);
        }

        template <typename Component>
        ComponentId RegisterComponent() { return componentStorage.Register<Component>(); }

        void Tick();

        template <typename Callable>
        void Each(Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            queryImpl<ArgList>(eastl::forward<Callable>(callable), eastl::make_index_sequence<ArgList::Count>());
        }

        World() { RegisterComponent<EntityId>(); }

    private:
        template <typename U>
        friend struct EventBuilder;

        template <typename C, typename T>
        friend struct EntityBuilder;

        friend struct Query;

        // Remove components should be sorted
        template <typename Components, typename ArgsTuple>
        EntityId commit(EntityId entity, ComponentsView removeComponents, ArgsTuple&& args)
        {
            return commitImpl<Components>(entity, removeComponents, eastl::forward<ArgsTuple>(args), eastl::make_index_sequence<Components::Count>());
        }

        ComponentInfo getComponentInfo(ComponentId id)
        {
            ASSERT(componentStorage.find(id) != componentStorage.end());
            return componentStorage[id];
        }

        // Components should be sorted
        Archetype& getOrCreateArchetype(ArchetypeId archetypeId, ComponentsView components)
        {
            Archetype* archetype = nullptr;

            size_t chunkSizePower = 7; // TODO move it somewhere

            auto it = archetypesMap.find(archetypeId);
            if (it == archetypesMap.end())
            {
                // Todo this could be rid with iterator and getComponentInfo call under hood
                eastl::fixed_vector<ComponentInfo, 32> componentsInfo; // TODO magic number
                for (auto compIt = components.begin(); compIt != components.end(); ++compIt)
                    componentsInfo.emplace_back(eastl::move(getComponentInfo(*compIt)));

                auto archUniqPtr = eastl::make_unique<Archetype>(archetypeId, chunkSizePower, componentsInfo.begin(), componentsInfo.end());
                archetype = archUniqPtr.get();
                archetypesMap.emplace(archetypeId, eastl::move(archUniqPtr));
            }
            else
                archetype = it->second.get();

            return *archetype;
        }

        bool getArchetypeForEntity(EntityId entity, Archetype*& archetype, ArchetypeEntityIndex& index)
        {
            EntityRecord record;
            if (!entityStorage.Get(entity, record))
                return false;

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

        template <typename Component, typename ArgsTuple, size_t... Index>
        void constructComponent(std::byte* ptr, ArgsTuple&& args, eastl::index_sequence<Index...>)
        {
            new (ptr) Component {std::forward<decltype(std::get<Index>(args))>(std::get<Index>(args))...};
        }

        template <typename Components, typename ArgsTuple, size_t... Index>
        EntityId commitImpl(EntityId entity, ComponentsView removeComponents, ArgsTuple&& args, eastl::index_sequence<Index...>)
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

            if (getArchetypeForEntity(entity, from, fromIndex))
            {
                for (auto it = from->GetComponentsBegin(); it != from->GetComponentsEnd(); ++it)
                    components.push_back_unsorted(*it); // Components already sorted
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
            for (auto it = removeComponents.begin(); it < removeComponents.end(); ++it)
                components.erase(*it);

            ArchetypeId archetypeId = GetArchetypeIdForComponents(components.begin(), components.end());
            Archetype& to = getOrCreateArchetype(archetypeId, {components.begin(), components.end()});
            if (from == &to)
                return entity;

            ArchetypeEntityIndex index = from ? to.Mutate(entityStorage, *from, fromIndex) : to.Insert(entityStorage, entity);

            // Component data initialization
            (
                constructComponent<typename Components::template Get<Index>>(
                    to.GetComponentData(GetComponentId<typename Components::template Get<Index>>)->GetData(index),
                    eastl::move(std::get<Index>(args)),
                    eastl::make_index_sequence<std::tuple_size_v<std::decay_t<decltype(std::get<Index>(args))>>>()),
                ...);

            UNUSED(index);
            // TODO validate remove components and add/remove at some thime.

            return entity;
        }

        template <typename Callable>
        void query(const Ecs::Query& query, Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            queryImpl<ArgList>(query, eastl::forward<Callable>(callable), eastl::make_index_sequence<ArgList::Count>());
        }

        // TODO this could be moved to query
        template <typename ArgumentList, typename Callable, size_t... Index>
        void queryImpl(const Ecs::Query& query, Callable&& callable, eastl::index_sequence<Index...>)
        {
            const auto& requireComps = query.desc().require;
            // Todo check all args in callable persist in requireComps with std::includes

            // TODO CACHING !
            for (auto it = archetypesMap.begin(); it != archetypesMap.end(); it++)
            {
                const Archetype& archetype = *it->second;
                if (!archetype.HasComponents(requireComps.begin(), requireComps.end()))
                    continue;

                QueryArchetype::Query(eastl::forward<Callable>(callable), archetype);
            }
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

        // TODO Dublicate impl
        template <typename ArgumentList, typename Callable, size_t... Index>
        void queryImpl(Callable&& callable, const eastl::index_sequence<Index...>&)
        {
            // TODO cache this

            eastl::array<ComponentId, ArgumentList::Count> components = {GetComponentId<typename ArgumentList::template Get<Index>>...};
            eastl::quick_sort(components.begin(), components.end());

            // TODO CACHING !
            for (auto it = archetypesMap.begin(); it != archetypesMap.end(); it++)
            {
                const Archetype& archetype = *it->second;
                archetype.HasComponents(components.begin(), components.end());

                QueryArchetype::Query(eastl::forward<Callable>(callable), archetype);
            }
        }

        template <typename EventType>
        void emit(EventType&& event, const EventDescription& eventDesc) const;
        template <typename EventType>
        void emitImmediately(EventType&& event, const EventDescription& eventDesc) const;

        void broadcastEventImmediately(const Ecs::Event& event) const;
        void dispatchEventImmediately(EntityId entity, const Ecs::Event& event) const;

    private:
        EntityStorage entityStorage;
        EventStorage eventStorage;
        SystemStorage systemStorage;
        ComponentStorage componentStorage;
        ska::flat_hash_map<ArchetypeId, eastl::unique_ptr<Archetype>> archetypesMap;
    };

    template <typename EventType>
    void World::emit(EventType&& event, const EventDescription& eventDesc) const
    {
        static_assert(std::is_base_of<Ecs::event, EventType>::value, "EventType must derive from Event");
        eventStorage.Push(std::move(event), eventDesc);
    }

    template <typename EventType>
    void World::emitImmediately(EventType&& event, const EventDescription& eventDesc) const
    {
        static_assert(std::is_base_of<Ecs::Event, EventType>::value, "EventType must derive from Event");
        if (!eventDesc.entity)
            broadcastEventImmediately(std::move(event));
        else
            dispatchEventImmediately(eventDesc.entity, std::move(event));
    }

    template <>
    inline System World::Create<System>(SystemDescription&& desc)
    {
        systemStorage.Push(desc);
        return Ecs::System(*this, desc.hashName);
    }

    template <>
    inline Query World::Create<Query>(QueryDescription&& desc)
    {
        return Ecs::Query(*this, eastl::forward<QueryDescription>(desc));
    }
}