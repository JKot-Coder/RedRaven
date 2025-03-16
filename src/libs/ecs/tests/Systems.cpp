#include <catch2/catch_test_macros.hpp>
#include <ecs/Ecs.hpp>

using namespace RR::Ecs;

struct WorldFixture
{
    World world;
};

struct TestEvent : Event
{
    TestEvent() : RR::Ecs::Event(GetEventId<TestEvent>, sizeof(TestEvent)) { };
};

struct IntEvent : Event
{
    IntEvent(int value) : RR::Ecs::Event(GetEventId<IntEvent>, sizeof(IntEvent)), value(value) { };
    int value;
};

struct FloatEvent : Event
{
    FloatEvent(float value) : RR::Ecs::Event(GetEventId<FloatEvent>, sizeof(FloatEvent)), value(value) { };
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

TEST_CASE_METHOD(WorldFixture, "Broadcast event immediate", "[Event]")
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

TEST_CASE_METHOD(WorldFixture, "Broadcast event deffered", "[Event]")
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

TEST_CASE_METHOD(WorldFixture, "Unicast event immediate", "[Event]")
{
    const auto entt1 = world.Entity().Edit().Add<int>(1).Apply();
    const auto entt2 = world.Entity().Edit().Add<int>(2).Apply();
    const auto entt3 = world.Entity().Edit().Add<int>(3).Apply();
    const auto entt4 = world.Entity().Edit().Add<int>(4).Apply();

    int calls = 0;
    eastl::array<std::pair<EntityId, int>, 5> data;
    world.System().Require<int>().OnEvent<IntEvent, TestEvent>(
        [&](EntityId id, const Event& event) {
            data[calls++] = {id, event.Is<IntEvent>() ? event.As<IntEvent>().value : -1};
        });
    world.System().Require<float>().OnEvent<IntEvent>([]() { REQUIRE(false); });
    world.System().Require<int>().OnEvent<FloatEvent, TestEvent>(
        [&](EntityId id, const Event& event) {
            data[calls++] = {id, event.Is<FloatEvent>() ? event.As<FloatEvent>().value : -1};
        });

    world.Event<IntEvent>().Entity(entt3).EmitImmediately(IntEvent {1});
    world.Event<IntEvent>().Entity(entt2).EmitImmediately(IntEvent {2});
    world.Event<TestEvent>().Entity(entt4).EmitImmediately({});
    world.Event<FloatEvent>().Entity(entt1).EmitImmediately(FloatEvent {3});

    REQUIRE(calls == 5);
    REQUIRE(data[0].first == entt3.GetId());
    REQUIRE(data[0].second == 1);
    REQUIRE(data[1].first == entt2.GetId());
    REQUIRE(data[1].second == 2);
    REQUIRE(data[2].first == entt4.GetId());
    REQUIRE(data[2].second == -1);
    REQUIRE(data[3].first == entt4.GetId());
    REQUIRE(data[3].second == -1);
    REQUIRE(data[4].first == entt1.GetId());
    REQUIRE(data[4].second == 3);

    UNUSED(entt1, entt2, entt3, entt4);
}

