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
    world.System().OnEvent<TestEvent>([] { });
    world.System().OnEvent<TestEvent>([] { });
    world.System().OnEvent<TestEvent>([] { });
}

TEST_CASE_METHOD(WorldFixture, "System event", "[System]")
{
    world.Entity().Add<int>().Apply();
    world.Entity().Add<int>().Apply();

    int calls = 0;
    world.System().Require<int>().OnEvent<TestEvent>([&calls](EntityId id) { REQUIRE(id); calls++; });
    world.EmitImmediately<TestEvent>({});

    REQUIRE(calls == 2);
}

TEST_CASE_METHOD(WorldFixture, "Broadcast event immediate", "[Event]")
{
    world.Entity().Add<int>().Apply();

    int calls = 0;
    eastl::array<int, 4> data;
    world.System().Require<int>().OnEvent<IntEvent>(
        [&calls, &data](EntityId id, const IntEvent& event) {
            REQUIRE(id);
            data[calls++] = event.value;
        });

    world.EmitImmediately<IntEvent>(1);
    world.EmitImmediately<IntEvent>(2);
    world.EmitImmediately<IntEvent>(3);
    world.EmitImmediately<IntEvent>(4);

    REQUIRE(calls == 4);
    REQUIRE(data[0] == 1);
    REQUIRE(data[1] == 2);
    REQUIRE(data[2] == 3);
    REQUIRE(data[3] == 4);
}

TEST_CASE_METHOD(WorldFixture, "Broadcast event deffered", "[Event]")
{
    world.Entity().Add<int>().Apply();

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

    world.Emit(IntEvent {1});
    world.Emit(FloatEvent {2});
    world.Emit(FloatEvent {3});
    world.Emit(IntEvent {4});

    REQUIRE(calls == 0);

    world.ProcessDefferedEvents();

    REQUIRE(calls == 4);
    REQUIRE(data[0] == 1);
    REQUIRE(data[1] == 2);
    REQUIRE(data[2] == 3);
    REQUIRE(data[3] == 4);
}

TEST_CASE_METHOD(WorldFixture, "Unicast event", "[Event]")
{
    int calls = 0;

    // Test cache

    // archetype created first
    const auto entt1 = world.Entity().Add<int>(1).Apply();
    world.System().Require<int>().OnEvent<TestEvent>([&]() { calls++; });
    world.EmitImmediately<TestEvent>(entt1, {});

    // system created first
    world.System().Require<float>().OnEvent<TestEvent>([&]() { calls++; });
    const auto entt2 = world.Entity().Add<float>(1.0f).Apply();
    world.EmitImmediately<TestEvent>(entt2, {});

    REQUIRE(calls == 2);
}


TEST_CASE_METHOD(WorldFixture, "Unicast event immediate", "[Event]")
{
    const auto entt1 = world.Entity().Add<int>(1).Apply();
    const auto entt2 = world.Entity().Add<int>(2).Apply();
    const auto entt3 = world.Entity().Add<int>(3).Apply();
    const auto entt4 = world.Entity().Add<int>(4).Apply();

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

    world.EmitImmediately<IntEvent>(entt3, {1});
    world.EmitImmediately<IntEvent>(entt2, {2});
    world.EmitImmediately<TestEvent>(entt4, {});
    world.EmitImmediately<FloatEvent>(entt1, {3});

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
}

TEST_CASE_METHOD(WorldFixture, "Unicast event deffered", "[Event]")
{
    const auto entt1 = world.Entity().Add<int>(1).Apply();
    const auto entt2 = world.Entity().Add<int>(2).Apply();
    const auto entt3 = world.Entity().Add<int>(3).Apply();
    const auto entt4 = world.Entity().Add<int>(4).Apply();

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

    world.Emit<IntEvent>(entt3, {1});
    world.Emit<IntEvent>(entt2, {2});
    world.Emit<TestEvent>(entt4, {});
    world.Emit<FloatEvent>(entt1, {3});

    REQUIRE(calls == 0);

    world.ProcessDefferedEvents();

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
}

TEST_CASE_METHOD(WorldFixture, "OnAppear", "[Event]")
{
    world.Entity().Add<int>(1).Apply();

    int calls = 0;

    // TODO MULTIPLE REQUIRE AND CHECK DATA
    world.System().Require<int>().OnEvent<OnAppear>([&]() { calls++; });
    world.Entity().Add<int>(1).Apply();
    REQUIRE(calls == 1);

    const auto entity = world.Entity().Add<int>(1).Add<float>(1.0f).Apply();
    entity.Edit().Remove<float>().Apply();
    REQUIRE(calls == 2);

    entity.Edit().Remove<int>().Apply();
    entity.Edit().Add<int>().Apply();
    REQUIRE(calls == 3);

    world.EmptyEntity().Edit().Add<int>().Apply();
    REQUIRE(calls == 4);
}

TEST_CASE_METHOD(WorldFixture, "OnDissapear", "[Event]")
{
    world.Entity().Add<int>(1).Apply();

    int calls = 0;
    // TODO MULTIPLE REQUIRE AND CHECK DATA
    world.System().Require<int>().OnEvent<OnDissapear>([&]() { calls++; });
    const auto entt1 = world.Entity().Add<int>(1).Apply();
    REQUIRE(calls == 0);
    entt1.Destruct();
    REQUIRE(calls == 1);

    const auto entt2 = world.Entity().Add<int>(1).Add<float>(1.0f).Apply();
    entt2.Edit().Remove<float>().Apply();
    REQUIRE(calls == 1);

    entt2.Edit().Remove<int>().Apply();
    REQUIRE(calls == 2);
}