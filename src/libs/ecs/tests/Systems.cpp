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

struct IntEvent : Event
{
    IntEvent(int value) : RR::Ecs::Event(GetComponentId<IntEvent>, sizeof(IntEvent)), value(value) { };
    int value;
};

struct FloatEvent : Event
{
    FloatEvent(float value) : RR::Ecs::Event(GetComponentId<FloatEvent>, sizeof(FloatEvent)), value(value) { };
    float value;
};

TEST_CASE_METHOD(WorldFixture, "System create", "[System]")
{
    world.System().OnEvent<TestEvent>([]{});
    world.System().OnEvent<TestEvent>([]{});
    world.System().OnEvent<TestEvent>([]{});
}

TEST_CASE_METHOD(WorldFixture, "System event", "[System]")
{
    world.Entity().Edit().Add<int>().Apply();
    world.Entity().Edit().Add<int>().Apply();

    int calls = 0;
    world.System().Require<int>().OnEvent<TestEvent>([&calls](EntityId id){ REQUIRE(id); calls++; });
    world.Event<TestEvent>().EmitImmediately(TestEvent{});

    REQUIRE(calls == 2);
}

TEST_CASE_METHOD(WorldFixture, "System immediate  event data", "[System]")
{
    world.Entity().Edit().Add<int>().Apply();

    int calls = 0;
    eastl::array<int, 4> data;
    world.System().Require<int>().OnEvent<IntEvent>(
        [&calls, &data](EntityId id, const IntEvent& event) {
            REQUIRE(id);
            data[calls++] = event.value;
        });


    world.Event<IntEvent>().EmitImmediately(IntEvent {1});
    world.Event<IntEvent>().EmitImmediately(IntEvent {2});
    world.Event<IntEvent>().EmitImmediately(IntEvent {3});
    world.Event<IntEvent>().EmitImmediately(IntEvent {4});

    REQUIRE(calls == 4);
    REQUIRE(data[0] == 1);
    REQUIRE(data[1] == 2);
    REQUIRE(data[2] == 3);
    REQUIRE(data[3] == 4);
}

TEST_CASE_METHOD(WorldFixture, "System deffered event data", "[System]")
{
    world.Entity().Edit().Add<int>().Apply();

    int calls = 0;
    eastl::array<int, 4> data;
    world.System().Require<int>().OnEvent<IntEvent>(
        [&calls, &data](EntityId id, const IntEvent& event) {
            REQUIRE(id);
            data[calls++] = event.value;
        });

    world.System().Require<int>().OnEvent<FloatEvent>(
        [&calls, &data](EntityId id, const FloatEvent& event) {
            REQUIRE(id);
            data[calls++] = event.value;
        });

    world.Event<IntEvent>().Emit(IntEvent {1});
    world.Event<FloatEvent>().Emit(FloatEvent {2});
    world.Event<FloatEvent>().Emit(FloatEvent {3});
    world.Event<IntEvent>().Emit(IntEvent {4});

    world.ProcessDefferedEvents();

    REQUIRE(calls == 4);
    REQUIRE(data[0] == 1);
    REQUIRE(data[1] == 2);
    REQUIRE(data[2] == 3);
    REQUIRE(data[3] == 4);
}

