#pragma once

#include "ecs/Event.hpp"
#include "ecs/World.hpp"

namespace RR::Ecs
{
    template <typename EventType>
    struct EventBuilder
    {
    public:
        EventBuilder(World& world) : world(world) { }

        EventBuilder Entity(EntityId entity)
        {
            entityId = entity;
            return *this;
        }

        void Emit(EventType&& event)
        {
            static_assert(std::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
            world.emit(entityId, eastl::forward<EventType>(event));
        }

        void EmitImmediately(const EventType& event) const
        {
            static_assert(std::is_base_of_v<Ecs::Event, EventType>, "EventType must derive from Event");
            world.emitImmediately(entityId, event);
        }

    private:
        World& world;
        EntityId entityId;
    };

    template <typename E>
    EventBuilder<E> World::Event() { return EventBuilder<E>(*this); };
}