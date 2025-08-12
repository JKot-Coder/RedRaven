#pragma once


#include "ecs/EntityId.hpp"
#include "ecs/Event.hpp"
#include "common/ChunkAllocator.hpp"

namespace RR::Ecs
{
    class EventStorage final
    {
    public:
        EventStorage() = default;

        template <typename EventType>
        void Push(EntityId entityId, EventType&& event)
        {
            static_assert(std::is_base_of<Ecs::Event, EventType>::value, "EventType must derive from Event");

            Event* ptr = current->allocator.create<EventType>(eastl::forward<EventType>(event));
            current->events.push_back({entityId, ptr});
        }

        template<typename CallBack>
        void ProcessEvents(const CallBack &cb)
        {
            if(current->events.empty())
                return;

            // We swap storages to avoid race conditions
            // Events emmited during processing will be processed in next frame
            Reset();

            // After swap current is empty, so we actually process events from next storage
            for (const auto eventRecord : next->events)
                cb(eventRecord.first, *eventRecord.second);
        }

        void Reset()
        {
            eastl::swap(current, next);
            current->allocator.reset();
            current->events.clear();
        }

    private:
        struct Storage
        {
            static constexpr size_t InitialEventQueueSize = 1024*1024;

            Common::ChunkAllocator allocator{InitialEventQueueSize};
            eastl::vector<eastl::pair<EntityId, Event*>> events;
        };

        eastl::array<Storage, 2> storages;
        Storage* current = &storages[0];
        Storage* next = &storages[1];
    };
}