
#pragma once

namespace flecs {
    typedef uint64_t ecs_id_t;
    typedef ecs_id_t ecs_entity_t;
    using entity_t = ecs_entity_t;
}

namespace RR::Ecs
{
    using HashType = uint64_t;
    using IdT = flecs::ecs_id_t;
    using EntityT = flecs::entity_t;
    struct Entity;
    struct Event;
    struct World;

    struct SystemDescription;

    template <typename E>
    struct EventBuilder;
    template <typename... Components>
    struct SystemBuilder;
}