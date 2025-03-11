#include <catch2/catch_test_macros.hpp>
#include <ecs/Ecs.hpp>

using namespace RR::Ecs;

struct WorldFixture
{
    World world;
};

struct TestEvent : Event
{
    TestEvent() : RR::Ecs::Event(GetComponentId<TestEvent>, sizeof(TestEvent)) { };
};

TEST_CASE_METHOD(WorldFixture, "System create", "[System]")
{
    world.System().OnEvent<TestEvent>([]{});
    world.System().OnEvent<TestEvent>([]{});
    world.System().OnEvent<TestEvent>([]{});
}

TEST_CASE_METHOD(WorldFixture, "System create2", "[System]")
{
    world.Entity().Edit().Add<int>().Apply();
    world.Entity().Edit().Add<int>().Apply();

    int calls = 0;
    world.System().Require<int>().OnEvent<TestEvent>([&calls](EntityId id){ REQUIRE(id); calls++; });
    world.Event<TestEvent>().EmitImmediately(TestEvent{});

    REQUIRE(calls == 2);
}
