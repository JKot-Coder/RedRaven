#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/EntityId.hpp"
#include "common/LinearAllocator.hpp"

namespace RR::Ecs
{
    // For Vector4 in Event EVENT_ALIGNMENT must be changed and/or
    // implement aligment on allocation
    static constexpr int EVENT_ALIGNMENT = alignof(uint32_t);

    struct alignas(EVENT_ALIGNMENT)
    Event
    {
        using SizeType = uint16_t;
        SizeType size;

        constexpr Event(size_t size_) : size(static_cast<SizeType>(size_)) {
            ASSERT(size_ < (size_t)std::numeric_limits<SizeType>::max);
        };
    };

    struct EventDescription
    {
        EntityId eventId = {};
        EntityId entity = {};
    };

    class EventStorage : protected Common::LinearAllocator<EVENT_ALIGNMENT>
    {
    private:
        static constexpr int InitialEventQueueSize = 1024*1024;
        static constexpr int Aligment = EVENT_ALIGNMENT;

    public:
        EventStorage() : LinearAllocator(InitialEventQueueSize) {};

        template <typename EventType>
        void Push(EventType&& event, const EventDescription& eventDesc)
        {
            static_assert(std::is_base_of<Ecs::Event, EventType>::value, "EventType must derive from Event");
            static_assert(IsAlignedTo(sizeof(EventType), Aligment));
            static_assert(IsAlignedTo(sizeof(EntityId), Aligment));

            constexpr auto eventSize = sizeof(EventType);
            const auto headerSize = sizeof(EntityId);
            auto at = Allocate(eventSize + headerSize);

            ASSERT(IsAlignedTo(at, Aligment));

            new (at) EntityId(eventDesc.eventId);
                    //if constexpr ((T::staticFlags() & EVFLG_DESTROY) == 0)
            memcpy(static_cast<char*>(at) + headerSize, &event, eventSize);
            // TODO MEM MOVE?
        }

        template<typename CallBack>
        void ProcessEvents(const CallBack &cb)
        {
            for (const auto& page : pages_)
            {
                const auto allocated = page->GetAllocated();
                const auto data = (uint8_t*)page->GetData();
                size_t pos = 0;

                while (pos < allocated)
                {
                    const auto headerSize = sizeof(EntityId);
                    EntityId eventId = *(EntityId*)(data + pos);
                    pos += headerSize;

                    Ecs::Event& event = *(Ecs::Event*)(data + pos);
                    cb(eventId, event);
                    pos += event.size;
                }

                ASSERT(pos == allocated);
            }

            Reset();
        }
    };
}