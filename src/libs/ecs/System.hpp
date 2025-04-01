#pragma once
#include "ecs/ForwardDeclarations.hpp"
#include "EASTL/fixed_function.h"
#include "ecs/Event.hpp"
#include "ecs/Index.hpp"
#include "ecs/Query.hpp"
#include "ecs/View.hpp"

namespace RR::Ecs
{
    struct Event;
    struct World;
    struct SystemBuilder;

    struct SystemDescription
    {
        HashName hashName;
        static constexpr size_t FunctionSize = 64;
        eastl::fixed_function<FunctionSize, void(Ecs::World&, const Ecs::Event&, Ecs::EntityId, Ecs::MatchedArchetypeSpan)> onEvent;
        eastl::fixed_vector<EventId, 16> onEvents;
        eastl::fixed_vector<HashName, 8> before;
        eastl::fixed_vector<HashName, 8> after;
    };

    struct System
    {
        void Run() const;

    private:
        friend Ecs::World;
        friend Ecs::SystemBuilder;

        explicit System(Ecs::World& world, SystemId id) : world(&world), id(id) { };

    private:
        World* world; // TODO We don't use it for now.
        SystemId id;
    };
}