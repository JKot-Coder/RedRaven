#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/System.hpp"
#include "ecs/FunctionTraits.hpp"
// #include <flecs.h>

namespace RR::Ecs
{
/*
    template<typename ArgList, bool HasArg = (ArgList::Count > 0)>
    struct IsFirstArgEvent;

    template <typename ArgList>
    struct IsFirstArgEvent<ArgList, true>
    {
        using FirstArg = typename ArgList::template Get<0>;
        static constexpr bool Value =
            eastl::is_same_v<FirstArg, Event> ||
            eastl::is_base_of_v<Event, FirstArg>;
    };

    template <typename ArgList>
    struct IsFirstArgEvent<ArgList, false>
    {
        static constexpr bool Value = false;
    };

    namespace _
    {
        template <typename TypeListT>
        struct TrimFirstArg;

        template <typename First, typename... Rest>
        struct TrimFirstArg<TypeList<First, Rest...>>
        {
            using TrimmedList = TypeList<Rest...>;
        };
    }
    template <typename TypeListT>
    using TrimFirstArg = typename _::TrimFirstArg<TypeListT>::TrimmedList;
*/
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

        template <typename Callback, typename Event, typename... Args>
        static void callQuery(Query query, const Event& event, Callback&& callback, TypeList<Args...>)
        {
          //  query.ForEach([&event](Args... args) { callback(event, eastl::forward<Args>(args)...); });
         // query.ForEach();
            query.world.query(query, eastl::forward<Callback>(callback));
        }

        template <typename... EventTypes, typename Callback>
        System OnEvent(Callback&& callback) &&
        {
            static_assert(sizeof...(EventTypes) > 0, "At least one event type must be specified");
            static_assert((std::is_base_of_v<Event, EventTypes> && ...), "All event types must derive from ecs::Event");

            using ArgList = GetArgumentList<Callback>;

            // todo GET/CHECK event RAW TYPE
            (desc.onEvents.emplace_back(GetTypeId<EventTypes>), ...);

            UNUSED(callback);
            UNUSED(ArgList);
            // TODO simplicate this mess with calls
            // TODO performance note: to call onEvent we query system and we query system again in onEvent

              desc.onEvent = [cb = std::forward<Callback>(callback)](const Ecs::Event&, const Ecs::Query& query)  {
                  query.world.query(query, eastl::move(cb));
            };


     /*       desc.onEvent = [cb = std::forward<Callback>(callback)](const Event& event, Query query)  {
                callQuery(query, event, eastl::move(cb), ArgList {});
            };


       desc.onEvent = [cb = std::forward<Callback>(callback)](const Event& event, Query query)  {
                callQuery(query, event, eastl::move(cb), ArgList {});
            };*/
/*

            constexpr bool isFirstArgEvent = IsFirstArgEvent<ArgList>::Value;

            if constexpr (isFirstArgEvent)
            {
                using TrimmedArgList = TrimFirstArg<ArgList>;
                desc.onEvent = [cb = std::forward<Callback>(callback)](const Event& event, Query query)  {
                    callQuery(query, event, eastl::move(cb), TrimmedArgList {});
                };
            } else 
            {
                desc.onEvent = [cb = std::forward<Callback>(callback)](const Event&, Query query) {
                    callQuery(query, eastl::move(cb), ArgList {});
                };
            }*/

            return view.world.Create(eastl::move(desc), eastl::move(view));
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
        SystemDescription desc;
        View view;
    };

    /*
    template <typename T, typename TDesc, typename... Components>
    struct NodeBuilder
    {
        explicit NodeBuilder(World& world, const char* name = nullptr)
            : world_(&world)
        {
            UNUSED(world, name);
            //  ecs_entity_desc_t entity_desc = {};
            //  entity_desc.name = name;
            //   entity_desc.sep = "::";
            //   entity_desc.root_sep = "::";
            // desc_.entity = ecs_entity_init(world_->Flecs(), &entity_desc);
        }

                template <typename Func>
                T Each(Func&& func)
                {
                    using Delegate = typename flecs::_::each_delegate<
                        typename std::decay<Func>::type, Components...>;


                    UNUSED(func);
                  //  desc_.callback = Delegate::run;
                 //   desc_.callback_ctx = Delegate::make(FLECS_FWD(func));
                  //  desc_.callback_ctx_free = reinterpret_cast<
        //ecs_ctx_free_t>(flecs::_::free_obj<Delegate>);
                //    return world_->Init<T>(desc_);

                    return
                }

    protected:
        TDesc desc_ {};
        World* world_ = nullptr;
    };

    namespace
    {
        template <typename... Components>
        using SystemBuilderBase = NodeBuilder<System, SystemDescription, Components...>;
    }

    template <typename... Components>
    struct SystemBuilder final : public SystemBuilderBase<Components...>
    {
        using BaseClass = SystemBuilderBase<Components...>;

        SystemBuilder(World& world, const char* name)
            : BaseClass(world, name)
        {
            ASSERT(name);
            ASSERT(name[0] != 0); // Not empty
            desc().hashName = HashName(name);
            // flecs::_::sig<Components...>(world).populate(this);
        }

        template <typename... EventTypes, typename Callback>
        SystemBuilder& OnEvent(Callback&& callback)
        {
            static_assert(sizeof...(EventTypes) > 0, "At least one event type must be specified");
            static_assert((std::is_base_of_v<Event, EventTypes> && ...), "All event types must derive from ecs::Event");

            (desc().onEvents.emplace_back(GetTypeId<EventTypes>), ...);

            if constexpr (std::is_invocable_r_v<void, decltype(callback), const Event&>)
            {
                desc().onEvent = callback;
            }
            else
            {
                if constexpr (std::is_invocable_r_v<void, decltype(callback)>)
                {
                    desc().onEvent = [callback](const Event&) {
                        callback();
                    };
                }
                else
                {
                    using EventType = std::tuple_element_t<0, std::tuple<EventTypes...>>;
                    static_assert(std::is_invocable_r_v<void, decltype(callback), const EventType&>,
                                  "OnEvent callback must accept either (const Event&) or no parameters");
                    desc().onEvent = [callback](const Event& evt) {
                        callback(static_cast<const EventType&>(evt));
                    };
                }
            }

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
            desc().after.push_back(HashName(name));
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
        SystemDescription& desc() { return BaseClass::desc_; };
    };

    template <typename... Components>
    inline SystemBuilder<Components...> World::System(const char* name)
    {
        return SystemBuilder<Components...>(*this, name);
    }
    */
}