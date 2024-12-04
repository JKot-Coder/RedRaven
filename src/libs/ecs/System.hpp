#pragma once

#include "ecs/Hash.hpp"
//#include "ecs/World.hpp"
#include "ecs/ForwardDeclarations.hpp"

#include "EASTL/fixed_vector.h"
#include "EASTL/unordered_map.h"
#include "EASTL/atomic.h"

#include <common/threading/Mutex.hpp>
#include <string_view>
#include <flecs.h>

namespace RR::Ecs
{
    using HashSystemName = HashString<64>;
    using HashSystemType = HashType;

    struct SystemDescription : public ecs_system_desc_t
    {
        HashSystemName hashName;
        eastl::fixed_vector<HashSystemName, 8> before;
        eastl::fixed_vector<HashSystemName, 8> after;
        eastl::fixed_vector<EntityT, 16> onEvents;
    };

    struct Id
    {
        Id() : world_(nullptr), id_(0) { }

        IdT RawId() const { return id_; }
        operator IdT() const { return id_; }

        const World* World() const { return world_; }

    protected:
        /* World is optional, but guarantees that entity identifiers extracted from
         * the id are valid */
        const Ecs::World* world_;
        IdT id_;
    };

    struct EntityView : Id
    {
        /** Return the entity name.
         *
         * @return The entity name.
         */
        //std::string_view Name() const { return std::string_view(ecs_get_name(world_->Flecs().c_ptr(), id_)); }
    };

    struct Entity : EntityView
    {
        Entity() : EntityView() { }

        /** Wrap an existing entity id.
         *
         * @param world The world in which the entity is created.
         * @param id The entity id.
         */
        explicit Entity(const Ecs::World& world, EntityT id)
        {
            world_ = &world;
            id_ = id;
        }
    };

    struct System final : Entity
    {
        explicit System()
        {
            id_ = 0;
            world_ = nullptr;
        }

    private:
        friend struct Ecs::World;

        explicit System(const Ecs::World& world, IdT id)
        {
            id_ = id;
            world_ = &world;
        };
    };

    class SystemStorage
    {
    public:
        SystemStorage() { };

        void Push(const SystemDescription& systemDescription)
        {
            HashSystemType hash = systemDescription.hashName;

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
        eastl::unordered_map<HashSystemType, SystemDescription> descriptions;
        std::vector<SystemDescription*> systems;
    };
}