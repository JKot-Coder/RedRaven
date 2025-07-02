#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/FunctionTraits.hpp"
#include "ecs/IterationHelpers.hpp"
#include "ecs/System.hpp"
#include "ecs/ComponentTraits.hpp"

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
        [[nodiscard]] SystemBuilder& With()
        {
            view.With<Components...>();
            return *this;
        }

        template <typename... Components>
        [[nodiscard]] SystemBuilder& Without()
        {
            view.Without<Components...>();
            return *this;
        }

        template <typename... Args>
        [[nodiscard]] SystemBuilder& Require()
        {
            static_assert((IsTag<Args> && ...), "All order tokens must be tags");

            #ifdef ECS_ENABLE_CHEKS
                auto check = [&]([[maybe_unused]] auto id, [[maybe_unused]] auto name) {
                    ECS_VERIFY(std::find(desc.produce.begin(), desc.produce.end(), id)  == desc.produce.end(), "Token {} can't be produced and required at the same time.", name);
                };
                (check(GetComponentId<Args>, GetComponentName<Args>), ...);
            #endif

            (desc.require.emplace_back(GetComponentId<Args>), ...);
            return *this;
        }

        template <typename... Args>
        [[nodiscard]] SystemBuilder& Produce()
        {
            static_assert((IsTag<Args> && ...), "All order tokens must be tags");
            #ifdef ECS_ENABLE_CHEKS
                auto check = [&]([[maybe_unused]] auto id, [[maybe_unused]] auto name) {
                    ECS_VERIFY(std::find(desc.require.begin(), desc.require.end(), id)  == desc.require.end(), "Token {} can't be produced and required at the same time.", name);
                };
                (check(GetComponentId<Args>, GetComponentName<Args>), ...);
            #endif

            (desc.produce.emplace_back(GetComponentId<Args>), ...);
            return *this;
        }

        template <typename... Components>
        [[nodiscard]] SystemBuilder& Track()
        {
            (desc.tracks.insert(GetComponentId<Components>), ...);
            // Since we track components, we should iterate over archetypes which have these components.
            return With<Components...>();
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
            desc.callback = [cb = std::forward<Callback>(callback)](Ecs::World& world, Ecs::Event const* event, Ecs::ArchetypeEntitySpan span) {
                world.invokeForEntities(span, event, eastl::move(cb));
            };

            return view.world.createSystem(eastl::move(desc), eastl::move(view), eastl::move(name));
        }

    private:
        SystemDescription desc = {};
        HashName name;
        View view;
    };
}