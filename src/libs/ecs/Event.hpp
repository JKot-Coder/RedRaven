#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "common/LinearAllocator.hpp"

namespace RR::Ecs
{
    // For Vector4 in Event EVENT_ALIGNMENT must be changed and/or
    // implement aligment on allocation
    static constexpr int EVENT_ALIGNMENT = alignof(uint32_t);

    struct alignas(EVENT_ALIGNMENT) event
    {
        using SizeType = uint16_t;
        SizeType size;

        constexpr event(size_t size_) : size(static_cast<SizeType>(size_)) {
            ASSERT(size_ < (size_t)std::numeric_limits<SizeType>::max);
        };
    };

    struct event_description
    {
        entity_t eventId;
        entity_t entity = 0;
    };

    class event_storage : protected Common::LinearAllocator<EVENT_ALIGNMENT>
    {
    private:
        static constexpr int InitialEventQueueSize = 1024*1024;
        static constexpr int Aligment = EVENT_ALIGNMENT;

    public:
        event_storage() : LinearAllocator(InitialEventQueueSize) {};

        template <typename EventType>
        void Push(EventType&& event, const event_description& eventDesc)
        {
            static_assert(std::is_base_of<Ecs::event, EventType>::value, "EventType must derive from Event");
            static_assert(IsAlignedTo(sizeof(EventType), Aligment));
            static_assert(IsAlignedTo(sizeof(entity_t), Aligment));

            constexpr auto eventSize = sizeof(EventType);
            const auto headerSize = sizeof(entity_t);
            auto at = Allocate(eventSize + headerSize);

            ASSERT(IsAlignedTo(at, Aligment));

            new (at) entity_t(eventDesc.eventId);
                    //if constexpr ((T::staticFlags() & EVFLG_DESTROY) == 0)
            memcpy(static_cast<char*>(at) + headerSize, &event, eventSize);
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
                    const auto headerSize = sizeof(entity_t);
                    entity_t eventId = *(entity_t*)(data + pos);
                    pos += headerSize;

                    Ecs::event& event = *(Ecs::event*)(data + pos);
                    cb(eventId, event);
                    pos += event.size;
                }

                ASSERT(pos == allocated);
            }

            Reset();
        }
    };

}