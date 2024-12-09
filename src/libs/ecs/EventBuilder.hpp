#pragma once

#include "ecs/Event.hpp"
#include "ecs/World.hpp"

namespace RR::Ecs
{
    template <typename EventType>
    struct EventBuilder
    {
    public:
        EventBuilder(const World& world) : world(&world), description()
        {
            description.eventId = world.typeId<EventType>();
        }

        EventBuilder Entity(EntityT entity)
        {
            description.entity = entity;
            return *this;
        }

        void Emit(EventType&& event) const
        {
            static_assert(std::is_base_of<Ecs::Event, EventType>::value, "EventType must derive from Event");
            world->emit(std::move(event), description);
        }

        void EmitImmediately(EventType&& event) const
        {
            static_assert(std::is_base_of<Ecs::Event, EventType>::value, "EventType must derive from Event");
            world->emitImmediately(std::move(event), description);
        }

    private:
        const World* world;
        EventDescription description;
    };

    template <typename E>
    EventBuilder<E> World::Event() const { return EventBuilder<E>(*this); };
}