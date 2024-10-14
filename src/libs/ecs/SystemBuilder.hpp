#pragma once

#include "EASTL/fixed_vector.h"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/Hash.hpp"
#include <string_view>
#include <flecs.h>

namespace RR::Ecs
{
    using HashSystemName = HashString<64>;

    struct SystemDesctiption : public ecs_system_desc_t
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

    template <>
    inline System World::Init<System>(const SystemDesctiption& desc)
    {
        const auto id = ecs_system_init(Flecs(), &desc);
        return Ecs::System(*this, id);
    }

    template <typename T, typename TDesc, typename... Components>
    struct NodeBuilder
    {
        explicit NodeBuilder(World& world, const char* name = nullptr)
            : world_(&world)
        {
            ecs_entity_desc_t entity_desc = {};
            entity_desc.name = name;
            entity_desc.sep = "::";
            entity_desc.root_sep = "::";
            desc_.entity = ecs_entity_init(world_->Flecs(), &entity_desc);
        }

        template <typename Func>
        T Each(Func&& func)
        {
            using Delegate = typename flecs::_::each_delegate<
                typename std::decay<Func>::type, Components...>;

            desc_.callback = Delegate::run;
            desc_.callback_ctx = Delegate::make(FLECS_FWD(func));
            desc_.callback_ctx_free = reinterpret_cast<
                ecs_ctx_free_t>(flecs::_::free_obj<Delegate>);
            return world_->Init<T>(desc_);
        }

    protected:
        TDesc desc_ {};
        World* world_ = nullptr;
    };

    namespace
    {
        template <typename... Components>
        using SystemBuilderBase = NodeBuilder<System, SystemDesctiption, Components...>;
    }

    template <typename... Components>
    struct SystemBuilder final : public SystemBuilderBase<Components...>
    {
        using BaseClass = SystemBuilderBase<Components...>;

        SystemBuilder(World& world, const char* name)
            : BaseClass(world, name)
        {
            ASSERT(name);
            ASSERT(name[0] != 0); //Not empty
            desc().hash = HashSystemName(name);
            // flecs::_::sig<Components...>(world).populate(this);
        }

        template <typename... EventTypes>
        SystemBuilder& OnEvent()
        {
            static_assert((std::is_base_of<Event, EventTypes>::value && ...), "EventType must derive from Event");
            (desc().onEvents.emplace_back(flecs::type_id<EventTypes>()), ...);
            return *this;
        }

        template <typename... Args>
        SystemBuilder& After(Args&&... args)
        {
            (after(args), ...);
            return *this;
        }

        SystemBuilder& after(const System& system)
        {
            after(system.Name());
            return *this;
        }

        SystemBuilder& after(std::string_view name)
        {
            desc().after.push_back(HashSystemName(name));
            return *this;
        }

        template <typename... Args>
        SystemBuilder& Before(Args&&... args)
        {
            (Before(args), ...);
            return *this;
        }

        SystemBuilder& before(const System& system)
        {
            before(system.Name());
            return *this;
        }

        SystemBuilder& before(std::string_view name)
        {
            desc().before.push_back(HashSystemName(name));
            return *this;
        }

    private:
        SystemDesctiption& desc() { return BaseClass::desc_; };
    };

    template <typename... Components>
    inline SystemBuilder<Components...> World::System(const char* name)
    {
        return SystemBuilder<Components...>(*this, name);
    }
}