#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/EntityId.hpp"
#include "common/ChunkAllocator.hpp"

namespace RR::Ecs
{
    struct Event
    {
        using SizeType = uint16_t;

        constexpr Event(ComponentId id, size_t size) : id(id), size(static_cast<SizeType>(size)) {
            ASSERT(size < (size_t)std::numeric_limits<SizeType>::max);
        };

        template <typename T>
        T* As()
        {
            static_assert(eastl::is_base_of_v<Event, T>, "T should be derived from Event");
            bool convertable = GetComponentId<T> == id;
            ASSERT(convertable);
            return convertable ? static_cast<T*>(this) : nullptr;
        }

        template <typename T>
        const T* As() const
        {
            static_assert(eastl::is_base_of_v<Event, T>, "T should be derived from Event");
            bool convertable = GetComponentId<T> == id;
            ASSERT(convertable);
            return convertable ? static_cast<const T*>(this) : nullptr;
        }

        ComponentId id; // TODO replace with eventId;
        SizeType size;
    };

    class EventStorage final
    {
    private:
        static constexpr size_t InitialEventQueueSize = 1024*1024;

    public:
        EventStorage() : allocator(InitialEventQueueSize) {};

        template <typename EventType>
        void Push(EntityId entityId, EventType&& event)
        {
            static_assert(std::is_base_of<Ecs::Event, EventType>::value, "EventType must derive from Event");

            Event* ptr = allocator.allocateTyped<EventType>(eastl::forward<EventType>(event));
            events.push_back({entityId, ptr});
        }

        template<typename CallBack>
        void ProcessEvents(const CallBack &cb)
        {
            for (const auto eventRecord : events)
                cb(eventRecord.first, *eventRecord.second);

            Reset();
        }

        void Reset()
        {
            allocator.reset();
            events.clear();
        }

        private:
            Common::ChunkAllocator allocator;
            eastl::vector<eastl::pair<EntityId, Event*>> events;
    };
}