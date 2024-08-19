
#pragma once

namespace flecs {
    typedef uint64_t ecs_id_t;
    typedef ecs_id_t ecs_entity_t;
    using entity_t = ecs_entity_t;
}

namespace RR::Ecs
{
    using EntityId = flecs::entity_t;
    using EventId = flecs::entity_t;

    struct Event;
}