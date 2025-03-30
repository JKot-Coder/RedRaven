#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/FunctionTraits.hpp"
#include "ecs/System.hpp"

namespace RR::Ecs
{
    struct [[nodiscard]] SystemBuilder
    {
    private:
        friend struct Ecs::World;
        SystemBuilder(World& world) : view(world) { };

    public:
        template <typename... Components>
        SystemBuilder Require() &&
        {
            view.Require<Components...>();
            return *this;
        }

        template <typename... Components>
        SystemBuilder Exclude() &&
        {
            view.Exclude<Components...>();
            return *this;
        }

        template <typename... Args>
        SystemBuilder& After(Args&&... args) &&
        {
            (after(args), ...);
            return *this;
        }

        template <typename... Args>
        SystemBuilder& Before(Args&&... args) &&
        {
            (after(args), ...);
            return *this;
        }

        template <typename... EventTypes, typename Callback>
        System OnEvent(Callback&& callback) &&
        {
            static_assert(sizeof...(EventTypes) > 0, "At least one event type must be specified");
            static_assert((std::is_base_of_v<Event, EventTypes> && ...), "All event types must derive from ecs::Event");

            // using ArgList = GetArgumentList<Callback>;

            // todo GET/CHECK event RAW TYPE
            (desc.onEvents.emplace_back(GetEventId<EventTypes>), ...);

            desc.onEvent = [cb = std::forward<Callback>(callback)](Ecs::World& world, const Ecs::Event& event, Ecs::EntityId entityId, Ecs::MatchedArchetypeSpan archetypes) {
                if (!entityId)
                    world.query(archetypes, &event, eastl::move(cb));
                else
                    world.queryForEntity(entityId, event, eastl::move(cb));
            };

            return view.world.createSystem(eastl::move(desc), eastl::move(view));
        }

    private:
        SystemBuilder& after(const System& system)
        {
            after(system.id);
            return *this;
        }

        SystemBuilder& after(const SystemId& system)
        {
            desc.after.push_back(system);
            return *this;
        }

        SystemBuilder& before(const System& system)
        {
            before(system.id);
            return *this;
        }

        SystemBuilder& before(const SystemId& system)
        {
            desc.before.push_back(system);
            return *this;
        }

    private:
        SystemDescription desc = {};
        View view;
    };
}