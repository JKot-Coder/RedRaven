#pragma once


#include "ecs/Hash.hpp"
//#include "ecs/World.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/TypeTraits.hpp"

#include "EASTL/fixed_vector.h"
#include "EASTL/unordered_map.h"
#include "EASTL/atomic.h"
#include "EASTL/fixed_function.h"

#include <common/threading/Mutex.hpp>
#include <string_view>

namespace RR::Ecs
{
    struct SystemDescription
    {
        static constexpr size_t FunctionSize = 64;
        using OnEventCallbackFunc = void(*)(const Event&);

        HashName hashName;
        eastl::fixed_function<FunctionSize, void(const Event&)> onEvent;
        eastl::fixed_vector<HashName, 8> before;
        eastl::fixed_vector<HashName, 8> after;
        eastl::fixed_vector<TypeId, 16> onEvents;
    };

    struct System final
    {
    private:
        friend struct Ecs::World;

        explicit System(const Ecs::World& world, HashName hashName)
        {
            hashName_ = hashName;
            world_ = &world;
        };

        HashName hashName_;
        const Ecs::World* world_ = nullptr;
    };

    class SystemStorage
    {
    public:
        SystemStorage() { };

        void Push(const SystemDescription& systemDescription)
        {
            HashType hash = systemDescription.hashName;

            Common::Threading::UniqueLock<Common::Threading::Mutex> lock(mutex);

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
        eastl::atomic<bool> isDirty = false;
        Common::Threading::Mutex mutex;
        eastl::unordered_map<HashType, SystemDescription> descriptions;
        std::vector<SystemDescription*> systems;
    };
}