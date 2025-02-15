#pragma once

#include "ecs/Archetype.hpp"
#include "ecs/EntityBuilder.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/EntityStorage.hpp"
#include "ecs/Event.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/FunctionTraits.hpp"
#include "ecs/IterationHelpers.hpp"
#include "ecs/System.hpp"
#include "ecs/TypeStorage.hpp"
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
        auto Entity() { return EntityBuilder<void, void>(*this); }

        template <typename T, typename DescriptionType>
        T Init(const DescriptionType& desc);

        void Tick();

        template <typename Callable>
        void Each(Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            queryImpl<ArgList>(eastl::forward<Callable>(callable), eastl::make_index_sequence<ArgList::Count>());
        }

    private:
        template <typename U>
        friend struct EventBuilder;

        template <typename C, typename T>
        friend class EntityBuilder;

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

        template <typename Tuple, std::size_t... Indices>
        void registerTypesInTuple()
        {
            // TODO for what?
            return typeStorage.Register<eastl::tuple_element_t<Indices, eastl::decay_t<Tuple>>::Component...>();
        }

        EntityId createEntity() { return EntityId(0, 0); }

        template <typename ComponentsList, typename ArgsTuple>
        EntityId createEntity(ArgsTuple&& args)
        {
            auto entity = createEntity();
            initComponents<ComponentsList, ArgsTuple>(entity, eastl::move(args));
            return entity;
        }

        template <typename ComponentsList, typename ArgsTuple, size_t... Index>
        void initComponentsImpl(EntityId entity, ArgsTuple&& argsTuple, const eastl::index_sequence<Index...>&)
        {
            constexpr ArchetypeId archetypeId = getArhetypeIdForComponents(ComponentsList {});
            Log::Format::Fatal("ArchetypeInfo: {}\n", eastl::hash<ArchetypeId> {}(archetypeId));
            Log::Format::Fatal("ArchetypeInfo: {}\n", archetypeId.Value());

            size_t chunkSizePower = 7; // TODO move it somewhere

            auto componentsInfo = getComponentsInfoArray(ComponentsList {});

            auto it = archetypesMap.find(archetypeId);
            Archetype* archetype = nullptr;

            if (it == archetypesMap.end())
            {
                auto archUniqPtr = eastl::make_unique<Archetype>(chunkSizePower, componentsInfo.begin(), componentsInfo.end());
                archetype = archUniqPtr.get();
                archetypesMap.emplace(archetypeId, eastl::move(archUniqPtr));
            }
            else
                archetype = it->second.get();

            const auto entityIndex = archetype->Insert();
            archetype->InitComponent(entityIndex, componentsInfo[Index]..., eastl::move(eastl::get<Index>(argsTuple))...);
        }

        template <typename ComponentsList, typename ArgsTuple>
        void initComponents(EntityId entity, ArgsTuple&& argsTuple)
        {
            initComponentsImpl<ComponentsList>(entity, eastl::forward<ArgsTuple>(argsTuple), eastl::make_index_sequence<ComponentsList::Count>());
        }

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
        TypeStorage typeStorage;
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
    inline System World::Init<System>(const SystemDescription& desc)
    {
        systemStorage.Push(desc);
        return Ecs::System(*this, desc.hashName);
    }

    void EntityBuilder<void, void>::Commit() && { world_.createEntity(); };
    template <typename ComponentsList, typename TupleType>
    void EntityBuilder<ComponentsList, TupleType>::Commit() && { world_.createEntity<ComponentsList>(eastl::move(args_)); };
}