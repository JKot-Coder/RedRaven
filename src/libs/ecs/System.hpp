#pragma once

#include "ecs/Hash.hpp"
#include "ecs/ForwardDeclarations.hpp"

#include "EASTL/fixed_vector.h"

#include <string_view>
#include <flecs.h>

namespace RR::Ecs
{
     using HashSystemName = HashString<64>;

    struct SystemDescription : public ecs_system_desc_t
    {
        HashSystemName hash;
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
        std::string_view Name() const { return std::string_view(ecs_get_name(world_->Flecs().c_ptr(), id_)); }
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
        SystemStorage()  {};

        template <typename EventType>
        void Push(EventType&& event, const EventDescription& eventDesc)
        {
            static_assert(std::is_base_of<Ecs::event, EventType>::value, "EventType must derive from Event");
            static_assert(IsAlignedTo(sizeof(EventType), Aligment));
            static_assert(IsAlignedTo(sizeof(EntityT), Aligment));

            constexpr auto eventSize = sizeof(EventType);
            const auto headerSize = sizeof(EntityT);
            auto at = Allocate(eventSize + headerSize);

            ASSERT(IsAlignedTo(at, Aligment));

            new (at) EntityT(eventDesc.eventId);
                    //if constexpr ((T::staticFlags() & EVFLG_DESTROY) == 0)
            memcpy(static_cast<char*>(at) + headerSize, &event, eventSize);
        }


        private:
            std::vector<SystemDescription2*> systemsToRegister;
            std::vector<SystemDescription*> systems;
    };

    template <>
    inline System World::Init<System>(const SystemDescription& desc)
    {
        const auto id = ecs_system_init(Flecs(), &desc);
        return Ecs::System(*this, id);
    }
}