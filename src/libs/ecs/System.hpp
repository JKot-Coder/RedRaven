#pragma once
/*
#include "ecs/Hash.hpp"
//
#include "ecs/Archetype.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/TypeTraits.hpp"

#include "EASTL/atomic.h"

#include "EASTL/unordered_map.h"

#include "ecs/Query.hpp"

#include <common/threading/Mutex.hpp>
#include <string_view>

*/
#include "EASTL/fixed_function.h"
#include "ecs/Index.hpp"
#include "ecs/Query.hpp"
#include "ecs/View.hpp"

namespace RR::Ecs
{
    struct Event;
    struct World;
    struct SystemBuilder;

    using SystemId = Index<struct SystemIdTag, HashType>; // TODO size_t ?

    struct SystemDescription
    {
        static constexpr size_t FunctionSize = 64;
        eastl::fixed_function<FunctionSize, void(Ecs::World& world, const Ecs::Event&, Ecs::MatchedArchetypeSpan)> onEvent;
        eastl::fixed_vector<ComponentId, 16> onEvents;
        eastl::fixed_vector<SystemId, 8> before;
        eastl::fixed_vector<SystemId, 8> after;
    };

    struct System
    {
    private:
        friend Ecs::World;
        friend Ecs::SystemBuilder;

        explicit System(Ecs::World& world, SystemId id) : world(world), id(id) { };

    private:
        World& world;
        SystemId id;
    };

    /*
        class SystemStorage
        {
        public:
            SystemStorage() { };


            void Push(const SystemDescription& systemDescription)
            {
                HashType hash = systemDescription.hashName;

                if (descriptions.find(hash) != descriptions.end())
                {
                    LOG_ERROR("System: {} already registered!", systemDescription.hashName.string.c_str());
                    return;
                }

                isDirty = true;
                descriptions[hash] = systemDescription;
            }

            void RegisterDeffered();

        private:
            eastl::atomic<bool> isDirty;
            eastl::unordered_map<HashType, SystemDescription> descriptions;
            std::vector<SystemDescription*> systems;
        };*/
}