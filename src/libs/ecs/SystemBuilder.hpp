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
        SystemBuilder(World& world, const HashName& name) : view(world)
        {
            desc.hashName = name;
        };

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

        template <typename... EventTypes, typename Callback>
        System OnEvent(Callback&& callback)
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
        SystemBuilder& after(System system)
        {
            after(system.id);
            return *this;
        }

        SystemBuilder& after(SystemId system)
        {
            after(view.world.GetSystemName(system));
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
            before(view.world.GetSystemName(system));
            return *this;
        }

        SystemBuilder& before(HashName&& name)
        {
            desc.before.emplace_back(eastl::forward<HashName>(name));
            return *this;
        }

    private:
        SystemDescription desc = {};
        View view;
    };
}