#include "ecs/Ecs.hpp"

namespace
{
    struct WorldFixture
    {
        RR::Ecs::World world;
    };

    template <typename Callable, typename CallableCheck>
    void immediateTest(Callable&& call, CallableCheck&& check)
    {
        RR::Ecs::World world;
        call(world);
        check(world);
    }

    template <typename Callable, typename CallableCheck>
    void defferedTest(Callable&& call, CallableCheck&& check)
    {
        RR::Ecs::World world;

        struct SingleExecutionToken
        {
        };
        world.Entity().Add<SingleExecutionToken>().Apply();

        bool called = false;
        world.View().With<SingleExecutionToken>().ForEach([&] {
            call(world);
            called = true;
        });

        check(world);
        REQUIRE(called);
    }
}

template <typename T>
struct SingletonComponent {
    ECS_SINGLETON;
    T x;
};