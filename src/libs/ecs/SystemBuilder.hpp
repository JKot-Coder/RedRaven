#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/FunctionTraits.hpp"
#include "ecs/System.hpp"
#include "ecs/World.hpp"

namespace RR::Ecs
{
    struct [[nodiscard]] SystemBuilder
    {
    private:
        friend struct Ecs::World;

        SystemBuilder(World& world) : view(world) { };
        SystemBuilder(World& world, const HashName& name) : name(name), view(world)  { };

    public:
        template <typename... Components>
        [[nodiscard]] SystemBuilder& Require()
        {
            view.Require<Components...>();
            return *this;
        }

        template <typename... Components>
        [[nodiscard]] SystemBuilder& Exclude()
        {
            view.Exclude<Components...>();
            return *this;
        }

        template <typename... Args>
        [[nodiscard]] SystemBuilder& After(Args&&... args)
        {
            (after(eastl::forward<Args>(args)), ...);
            return *this;
        }

        template <typename... Args>
        [[nodiscard]] SystemBuilder& Before(Args&&... args)
        {
            (before(eastl::forward<Args>(args)), ...);
            return *this;
        }

        template <typename... EventTypes>
        [[nodiscard]] SystemBuilder& OnEvent()
        {
            static_assert(sizeof...(EventTypes) > 0, "At least one event type must be specified");
            static_assert((std::is_base_of_v<Event, EventTypes> && ...), "All event types must derive from ecs::Event");

            // todo GET/CHECK event RAW TYPE
            (desc.onEvents.emplace_back(GetEventId<EventTypes>), ...);

            return *this;
        }

        template <typename Callback>
        System ForEach(Callback&& callback)
        {
#ifdef ENABLE_ASSERTS
            Debug::ValidateLambdaArgumentsAgainstView(this->view, callback);
#endif
            desc.callback = [cb = std::forward<Callback>(callback)](Ecs::World& world, Ecs::Event const * event, Ecs::EntityId entityId, Ecs::MatchedArchetypeSpan archetypes) {
                if (!entityId)
                    world.query(archetypes, event, eastl::move(cb));
                else
                    world.queryForEntity(entityId, event, eastl::move(cb));
            };

            return view.world.createSystem(eastl::move(desc), eastl::move(view), eastl::move(name));
        }

    private:
        SystemBuilder& after(System system)
        {
            after(system.id);
            return *this;
        }

        SystemBuilder& after(SystemId system)
        {
            after(fmt::format("system_{}", system.GetRaw()));
            return *this;
        }

        SystemBuilder& after(HashName&& name)
        {
            desc.after.emplace_back(eastl::forward<HashName>(name));
            return *this;
        }

        SystemBuilder& before(System system)
        {
            before(system.id);
            return *this;
        }

        SystemBuilder& before(SystemId system)
        {
            before(fmt::format("system_{}", system.GetRaw()));
            return *this;
        }

        SystemBuilder& before(HashName&& name)
        {
            desc.before.emplace_back(eastl::forward<HashName>(name));
            return *this;
        }

    private:
        SystemDescription desc = {};
        HashName name;
        View view;
    };
}