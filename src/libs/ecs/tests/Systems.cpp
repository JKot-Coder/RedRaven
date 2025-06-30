#include <catch2/catch_all.hpp>
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
    world.System().ForEach([] { });
    world.System().OnEvent<TestEvent>().ForEach([] { });
    world.System().With<int>().OnEvent<TestEvent>().ForEach([] { });
}

TEST_CASE_METHOD(WorldFixture, "System lock", "[System]")
{
    REQUIRE(!world.IsLocked());
    struct Foo { int x; };
    world.Entity().Add<Foo>(1).Apply();

    bool called = false;
    const auto system = world.System().With<Foo>().ForEach([&] {
        REQUIRE(world.IsLocked());
        called = true;
    });

    system.Run();

    REQUIRE(called);
    REQUIRE(!world.IsLocked());
}

TEST_CASE_METHOD(WorldFixture, "System event", "[System]")
{
    world.Entity().Add<int>().Apply();
    world.Entity().Add<int>().Apply();

    int calls = 0;
    world.System().With<int>().OnEvent<TestEvent>().ForEach([&calls](EntityId id) { REQUIRE(id); calls++; });
    world.OrderSystems();
    world.EmitImmediately<TestEvent>({});

    REQUIRE(calls == 2);
}

TEST_CASE_METHOD(WorldFixture, "Broadcast event immediate", "[Event]")
{
    world.Entity().Add<int>().Apply();

    int calls = 0;
    eastl::array<int, 4> data;
    world.System().With<int>().OnEvent<IntEvent>().ForEach(
        [&calls, &data](EntityId id, const IntEvent& event) {
            REQUIRE(id);
            data[calls++] = event.value;
        });
    world.OrderSystems();

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
    world.System().With<int>().OnEvent<IntEvent>().ForEach(
        [&calls, &data](EntityId id, const IntEvent& event) {
            REQUIRE(id);
            data[calls++] = event.value;
        });

    world.System().With<int>().OnEvent<FloatEvent>().ForEach(
        [&calls, &data](EntityId id, const FloatEvent& event) {
            REQUIRE(id);
            data[calls++] = event.value;
        });

    world.OrderSystems();
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
    world.System().With<int>().OnEvent<TestEvent>().ForEach([&]() { calls++; });
    world.OrderSystems();
    world.EmitImmediately<TestEvent>(entt1, {});

    // system created first
    world.System().With<float>().OnEvent<TestEvent>().ForEach([&]() { calls++; });
    world.OrderSystems();
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
    world.System().With<int>().OnEvent<IntEvent, TestEvent>().ForEach(
        [&](EntityId id, const Event& event) {
            data[calls++] = {id, event.Is<IntEvent>() ? event.As<IntEvent>().value : -1};
        });
    world.System().With<float>().OnEvent<IntEvent>().ForEach([]() { REQUIRE(false); });
    world.System().With<int>().OnEvent<FloatEvent, TestEvent>().ForEach(
        [&](EntityId id, const Event& event) {
            data[calls++] = {id, event.Is<FloatEvent>() ? event.As<FloatEvent>().value : -1};
        });

    world.OrderSystems();
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
    world.System().With<int>().OnEvent<IntEvent, TestEvent>().ForEach(
        [&](EntityId id, const Event& event) {
            data[calls++] = {id, event.Is<IntEvent>() ? event.As<IntEvent>().value : -1};
        });
    world.System().With<float>().OnEvent<IntEvent>().ForEach([]() { REQUIRE(false); });
    world.System().With<int>().OnEvent<FloatEvent, TestEvent>().ForEach(
        [&](EntityId id, const Event& event) {
            data[calls++] = {id, event.Is<FloatEvent>() ? event.As<FloatEvent>().value : -1};
        });

    world.OrderSystems();
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
    SECTION("Simple")
    {
        world.Entity().Add<int>(1).Apply();

        int calls = 0;
        world.System().With<int>().OnEvent<OnAppear>().ForEach([&]() { calls++; });
        world.OrderSystems();
        world.Entity().Add<int>(1).Apply();
        REQUIRE(calls == 1);

        const auto entity = world.Entity().Add<int>(1).Add<float>(1.0f).Apply();
        REQUIRE(calls == 2);
        entity.Edit().Remove<float>().Apply();
        REQUIRE(calls == 2);

        entity.Edit().Remove<int>().Apply();
        REQUIRE(calls == 2);
        entity.Edit().Add<int>().Apply();
        REQUIRE(calls == 3);

        world.EmptyEntity().Edit().Add<int>().Apply();
        REQUIRE(calls == 4);
    }
    SECTION("Without")
    {
        int calls = 0;
        world.System().With<int>().Without<float>().OnEvent<OnAppear>().ForEach([&]() { calls++; });
        world.OrderSystems();
        world.Entity().Add<int>(1).Add<float>(1.0f).Apply();
        REQUIRE(calls == 0);

        const auto entt2 = world.Entity().Add<int>(1).Add<float>(1.0f).Apply();
        entt2.Edit().Remove<float>().Apply();
        REQUIRE(calls == 1);

        entt2.Edit().Remove<int>().Apply();
        REQUIRE(calls == 1);
    }
}

TEST_CASE_METHOD(WorldFixture, "OnDissapear", "[Event]")
{
    SECTION("Simple")
    {
        world.Entity().Add<int>(1).Apply();

        int calls = 0;
        world.System().With<int>().OnEvent<OnDissapear>().ForEach([&]() { calls++; });
        world.OrderSystems();
        const auto entt1 = world.Entity().Add<int>(1).Apply();
        REQUIRE(calls == 0);
        entt1.Destroy();
        REQUIRE(calls == 1);

        const auto entt2 = world.Entity().Add<int>(1).Add<float>(1.0f).Apply();
        entt2.Edit().Remove<float>().Apply();
        REQUIRE(calls == 1);

        entt2.Edit().Remove<int>().Apply();
        REQUIRE(calls == 2);
    }
    SECTION("Without")
    {
        int calls = 0;
        world.System().With<int>().Without<float>().OnEvent<OnDissapear>().ForEach([&]() { calls++; });
        world.OrderSystems();
        const auto entt1 = world.Entity().Add<int>(1).Add<float>(1.0f).Apply();
        entt1.Destroy();
        REQUIRE(calls == 0);

        const auto entt2 = world.Entity().Add<int>(1).Add<float>(1.0f).Apply();
        entt2.Edit().Remove<float>().Apply();
        REQUIRE(calls == 0);

        entt2.Edit().Remove<int>().Apply();
        REQUIRE(calls == 1);
    }
}

TEST_CASE("SystemOrder", "[System]")
{
    struct System1 { };
    struct System2 { };
    struct System3 { };
    struct System4 { };
    struct System5 { };

    using Results = eastl::vector<int>;
    const auto check = [](World& world) {
        bool called = false;
        const auto checkResults = world.System().With<Results>().ForEach([&](Results& results)
        {
            called = true;
            REQUIRE(results.size() == 5);
            REQUIRE(results[0] == 0);
            REQUIRE(results[1] == 1);
            REQUIRE(results[2] == 2);
            REQUIRE(results[3] == 3);
            REQUIRE(results[4] == 4);
            results.clear();
        });

        world.OrderSystems();

        const auto resultsEntt = world.Entity().Add<Results>().Apply();
        // Check system order called on unicast event
        world.EmitImmediately<TestEvent>(resultsEntt, {});
        checkResults.Run();
        REQUIRE(called);
        called = false;
        UNUSED(resultsEntt);

        // Check system order called on broadcast event
        world.EmitImmediately<TestEvent>({});
        checkResults.Run();
        REQUIRE(called);
    };
    SECTION("order")
    {
        World world;
        world.System("system1").Produce<System1>().Require<System2>().With<Results>().OnEvent<TestEvent>().ForEach([&](Results& results) { results.push_back(4); });
        world.System("system2").Produce<System2>().Require<System3>().With<Results>().OnEvent<TestEvent>().ForEach([&](Results& results) { results.push_back(3); });
        world.System("system3").Produce<System3>().Require<System4>().With<Results>().OnEvent<TestEvent>().ForEach([&](Results& results) { results.push_back(2); });
        world.System("system4").Produce<System4>().Require<System5>().With<Results>().OnEvent<TestEvent>().ForEach([&](Results& results) { results.push_back(1); });
        world.System("system5").Produce<System5>().With<Results>().OnEvent<TestEvent>().ForEach([&](Results& results) { results.push_back(0); });
        check(world);
    }
    SECTION("Mixed")
    {
        World world;
        //         --- 3 - 1
        //        /     \ /
        // 5 --- 4 ----- 2
        world.System("system1").Produce<System1>().Require<System2>().Require<System3>().With<Results>().OnEvent<TestEvent>().ForEach([&](Results& results) { results.push_back(4); });
        world.System("system2").Produce<System2>().Require<System3>().Require<System4>().With<Results>().OnEvent<TestEvent>().ForEach([&](Results& results) { results.push_back(3); });
        world.System("system3").Produce<System3>().Require<System4>().With<Results>().OnEvent<TestEvent>().ForEach([&](Results& results) { results.push_back(2); });
        world.System("system4").Produce<System4>().Require<System5>().With<Results>().OnEvent<TestEvent>().ForEach([&](Results& results) { results.push_back(1); });
        world.System("system5").Produce<System5>().With<Results>().OnEvent<TestEvent>().ForEach([&](Results& results) { results.push_back(0); });
        check(world);
    }
}

TEST_CASE_METHOD(WorldFixture, "Dependency cycle", "[System]")
{
    struct System1 { };
    struct System2 { };
    struct System3 { };

    world.System("system1").Produce<System1>().Require<System2>().ForEach([&]() { });
    world.System("system2").Produce<System2>().Require<System3>().ForEach([&]() { });
    world.System("system3").Produce<System3>().Require<System1>().ForEach([&]() {  });
    REQUIRE_THROWS_WITH(world.OrderSystems(), "ES <system3> in graph to become cyclic and was removed from sorting. ES order is non-deterministic.");
}

TEST_CASE_METHOD(WorldFixture, "Require token that is never produced by any system", "[System]")
{
    struct System1 { };
    struct System2 { };

    world.System("system1").Produce<System1>().Require<System2>().ForEach([&]() { });;
    REQUIRE_THROWS_WITH(world.OrderSystems(), "ES <system1> is require token that is never produced by any system.");
}
