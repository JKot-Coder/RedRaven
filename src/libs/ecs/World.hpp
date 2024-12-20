#pragma once

#include <flecs.h>
#include "Event.hpp"
#include "System.hpp"
#include "ecs/ForwardDeclarations.hpp"

namespace RR::Ecs
{
    struct World
    {
    public:
        template <typename EventType>
        inline EventBuilder<EventType> Event() const;
        template <typename... Components>
        inline SystemBuilder<Components...> System(const char* name);

        template <typename T, typename DescriptionType>
        T Init(const DescriptionType& desc);

        void Tick();
        const flecs::world& Flecs() const { return world; }

        /** Lookup entity by name.
         *
         * @param name Entity name.
         * @param recursive When false, only the current scope is searched.
         * @result The entity if found, or 0 if not found.
         */
        Entity Lookup(const char *name, const char *sep = "::", const char *root_sep = "::", bool recursive = true) const;

        /** Get alive entity for id.
         * Returns the entity with the current generation.
         *
         * @see ecs_get_alive()
         */
        Entity GetAlive(EntityT e) const;

    private:
        template<typename U>
        friend struct EventBuilder;

        EntityT makeEntity(const ecs_entity_desc_t& desc) { return ecs_entity_init(world, &desc); }

        template <typename EventType>
        void emit(EventType&& event, const EventDescription& eventDesc) const;
        template <typename EventType>
        void emitImmediately(EventType&& event, const EventDescription& eventDesc) const;
        template <typename E>
        flecs::entity_t typeId() const { return flecs::type_id<E>(); };

        void broadcastEventImmediately(const Ecs::Event& event) const;
        void dispatchEventImmediately(EntityT entity, const Ecs::Event& event) const;

    private:
        flecs::world world;
        EventStorage eventStorage;
        SystemStorage systemStorage;
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
        if(!eventDesc.entity)
            broadcastEventImmediately(std::move(event));
        else
            dispatchEventImmediately(eventDesc.entity, std::move(event));
    }

    template <>
    inline System World::Init<System>(const SystemDescription& desc)
    {
        const auto id = ecs_system_init(Flecs(), &desc);
        systemStorage.Push(desc);
        return Ecs::System(*this, id);
    }
}