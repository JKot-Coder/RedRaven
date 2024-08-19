#pragma once

#include "Event.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include <flecs.h>

namespace RR::Ecs
{
    template <typename E>
    struct EventBuilder;

    class World
    {
    public:
        template <typename EventType>
        inline EventBuilder<EventType> Event();

        void Tick();

    private:
        template<typename U>
        friend struct EventBuilder;

        template <typename EventType>
        void emit(EventType&& event, const EventDescription& eventDesc);
        template <typename EventType>
        void emitImmediately(EventType&& event, const EventDescription& eventDesc);
        template <typename E>
        flecs::entity_t typeId() { return flecs::type_id<E>(); };

        void broadcastEventImmediately(const Ecs::Event& event) const;
        void dispatchEventImmediately(EntityId entity, const Ecs::Event& event) const;

    private:
        flecs::world world;
        EventStorage eventStorage;
    };


    template <typename EventType>
    void World::emit(EventType&& event, const EventDescription& eventDesc)
    {
        static_assert(std::is_base_of<Ecs::Event, EventType>::value, "EventType must derive from Event");
        eventStorage.Push(std::move(event), eventDesc);
    }

    template <typename EventType>
    void World::emitImmediately(EventType&& event, const EventDescription& eventDesc)
    {
        static_assert(std::is_base_of<Ecs::Event, EventType>::value, "EventType must derive from Event");
     /*   if(!eventDesc.entity)
            broadcastEventImmediately(std::move(event));
        else
            sendEventImmediately(eventDesc.entity, std::move(event));*/
    }

    template <typename E>
    void emit(const EventDescription& eventDesc);
    template <typename E>
    void emitImmediately(const EventDescription& eventDesc);

    template <typename EventType>
    struct EventBuilder
    {
    public:
        EventBuilder(World* world) : world(world), description()
        {
            ASSERT(world);
            description.eventId = world->typeId<EventType>();
        }

        EventBuilder Entity(EntityId entity) const
        {
            description.entity = entity;
            return *this;
        }

        void Emit(EventType&& event) const
        {
            static_assert(std::is_base_of<Event, EventType>::value, "EventType must derive from Event");
            world->emit(std::move(event), description);
        }

        void EmitImmediately(EventType&& event) const
        {
            static_assert(std::is_base_of<Event, EventType>::value, "EventType must derive from Event");
            world->emitImmediately(std::move(event), description);
        }

    private:
        World* world;
        EventDescription description;
    };

    template <typename E>
    EventBuilder<E> World::Event() { return EventBuilder<E>(this); };
}