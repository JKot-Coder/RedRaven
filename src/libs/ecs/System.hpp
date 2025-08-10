#pragma once

#include "ecs/Event.hpp"
#include "ecs/ForwardDeclarations.hpp"

#include "EASTL/fixed_function.h"

namespace RR::Ecs
{
    struct Event;
    struct World;
    struct SystemBuilder;

    struct SystemDescription
    {
        static constexpr size_t FunctionSize = 64;
        eastl::fixed_function<FunctionSize, void(Ecs::World&, Ecs::Event const *, ArchetypeEntitySpan span)> callback;
        eastl::fixed_vector<EventId, 16> onEvents;
        eastl::fixed_vector<Meta::ComponentId, 8> require;
        eastl::fixed_vector<Meta::ComponentId, 8> produce;
        Meta::ComponentsSet tracks;
    };

    struct System
    {
        void Run() const;

    private:
        friend Ecs::World;
        friend Ecs::SystemBuilder;

        explicit System(Ecs::World& world, SystemId id) : world(&world), id(id) { };

    private:
        World* world;
        SystemId id;
    };
}