#pragma once

#include "ecs/Archetype.hpp"
#include "ecs/EntityStorage.hpp"
#include "ecs/TypeStorage.hpp"
#include "ecs/Event.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/System.hpp"
#include "ecs/EntityBuilder.hpp"
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
        EntityBuilder<void> Entity(){ return EntityBuilder<void>(*this); }

        template <typename T, typename DescriptionType>
        T Init(const DescriptionType& desc);

        void Tick();

    private:
        template <typename U>
        friend struct EventBuilder;

        template <typename U>
        friend class EntityBuilder;

        template <typename Tuple, std::size_t... Indices>
        static constexpr ArchetypeId getArhetypeIdForTuple()
        {
            return ArchetypeInfo<typename eastl::tuple_element_t<Indices, eastl::decay_t<Tuple>>::Component...>::Id;
        }

        template <typename Tuple, std::size_t... Indices>
        void registerTypesInTuple()
        {
            return typeStorage.Register<eastl::tuple_element_t<Indices, eastl::decay_t<Tuple>>::Component...>();
        }

        EntityId createEntity() { return EntityId(0, 0); }

        template <typename TupleType>
        EntityId createEntity(TupleType&& args)
        {
            constexpr size_t numComponents = eastl::tuple_size<TupleType>();
            constexpr auto indexSequence = eastl::make_index_sequence<numComponents>();

            auto entity = createEntity();
            initComponents<TupleType>(entity, eastl::move(args), indexSequence);
            return entity;
        }

        template<typename Arg>
        void initComponent(Archetype<>& archetype, ArchetypeEntityIndex entity_index, Arg&& arg) {
            using ArgType = typename eastl::remove_reference<Arg>::type;
            using Component = typename ArgType::Component;
            constexpr size_t num_args = ArgType::num_args;
            initComponent<Component>(archetype, entity_index, arg.args, std::make_index_sequence<num_args>());
        }


        template <typename TupleType, size_t... Index>
        void initComponents(EntityId entity, TupleType&& tuple, const eastl::index_sequence<Index...>& indices)
        {
            UNUSED(entity);
            UNUSED(tuple);

            static constexpr ArchetypeId archetypeId = getArhetypeIdForTuple<TupleType, Index...>();
            Log::Format::Fatal("ArchetypeInfo: {}\n", eastl::hash<ArchetypeId>{}(archetypeId));
            Log::Format::Fatal("ArchetypeInfo: {}\n", archetypeId.Value());

            Archetype<>* archetype = nullptr;
            auto it = archetypesMap.find(archetypeId);
            if (it == archetypesMap.end())
            {
                //size_t chunkSizePower, ComponentsView componets)
            //    auto archUniqPtr = eastl::make_unique<Archetype<>>();
            //    archetypesMap.emplace(archetypeId, eastl::move(archUniqPtr));
                //archetype = archUniqPtr.get();
            }
            else
                archetype = it->second.get();

 UNUSED(archetype);
       //     const auto enitityIndex = archetype->insert(entity, componentId);

            // UNUSED(Index);
            // auto& archetype = getArchetype(mask, info);
            // const auto index = archetype.insert(entity, mask);
            // initComponent(archetype, index, eastl::get<Index>(eastl::move(tuple)))...;
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
        ska::flat_hash_map<ArchetypeId, eastl::unique_ptr<Archetype<>>> archetypesMap;
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

    void EntityBuilder<void>::Commit() && { world_.createEntity(); };
    template<typename TupleType>
    void EntityBuilder<TupleType>::Commit() && { world_.createEntity(eastl::move(args_)); };
}