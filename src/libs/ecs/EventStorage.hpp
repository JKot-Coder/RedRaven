#pragma once


#include "ecs/EntityId.hpp"
#include "ecs/Event.hpp"
#include "common/ChunkAllocator.hpp"

namespace RR::Ecs
{
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

            Event* ptr = allocator.create<EventType>(eastl::forward<EventType>(event));
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