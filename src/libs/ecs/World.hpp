#pragma once

#include <flecs.h>
#include "Event.hpp"
#include "ecs/ForwardDeclarations.hpp"

namespace RR::Ecs
{
    template <typename E>
    struct event_builder;
    template <typename... Components>
    struct system_builder;

    struct world
    {
    public:
        template <typename EventType>
        inline event_builder<EventType> event();
        template <typename... Components>
        inline system_builder<Components...> system();

        void tick();
        flecs::world& flecs() {return world;}

    private:
        template<typename U>
        friend struct event_builder;

        template <typename EventType>
        void emit(EventType&& event, const event_description& eventDesc);
        template <typename EventType>
        void emitImmediately(EventType&& event, const event_description& eventDesc);
        template <typename E>
        flecs::entity_t typeId() { return flecs::type_id<E>(); };

        void broadcastEventImmediately(const Ecs::event& event) const;
        void dispatchEventImmediately(entity_t entity, const Ecs::event& event) const;

    private:
        flecs::world world;
        event_storage eventStorage;
    };

    template <typename EventType>
    void world::emit(EventType&& event, const event_description& eventDesc)
    {
        static_assert(std::is_base_of<Ecs::event, EventType>::value, "EventType must derive from Event");
        eventStorage.Push(std::move(event), eventDesc);
    }

    template <typename EventType>
    void world::emitImmediately(EventType&& event, const event_description& eventDesc)
    {
        static_assert(std::is_base_of<Ecs::event, EventType>::value, "EventType must derive from Event");
        if(!eventDesc.entity)
            broadcastEventImmediately(std::move(event));
        else
            sendEventImmediately(eventDesc.entity, std::move(event));
    }
}