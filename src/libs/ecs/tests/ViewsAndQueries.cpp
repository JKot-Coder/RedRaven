#include <catch2/catch_test_macros.hpp>
#include <ecs/Ecs.hpp>

using namespace RR::Ecs;

struct WorldFixture
{
    World world;
};

// clang-format off
struct Foo { int x;};
struct Bar { int x;};
// clang-format on

TEST_CASE_METHOD(WorldFixture, "View read", "[View]")
{
    world.Entity();

    world.Entity().Edit().Add<Foo>(1).Apply();
    world.Entity().Edit().Add<Bar>(1).Apply();
    world.Entity().Edit().Add<Foo>(1).Apply();

    const auto view = world.View().Require<Foo>();

    int summ = 0;
    view.ForEach([&](Foo foo) { summ += foo.x; });

    REQUIRE(summ == 2);

    summ = 0;
    view.ForEach([&](Foo& foo) { summ += foo.x; });
    REQUIRE(summ == 2);

    summ = 0;
    view.ForEach([&](Foo* foo) { summ += foo->x; });
    REQUIRE(summ == 2);
}

TEST_CASE_METHOD(WorldFixture, "View modify", "[View]")
{
    world.Entity().Edit().Add<Foo>(1).Apply();
    world.Entity().Edit().Add<Foo>(1).Apply();

    const auto view = world.View().Require<Foo>();

    int summ = 0;
    view.ForEach([&](Foo foo) { summ += foo.x; });
    REQUIRE(summ == 2);

    view.ForEach([&](Foo& foo) { foo.x++; });
    view.ForEach([&](Foo* foo) { foo->x++; });

    summ = 0;
    view.ForEach([&](Foo foo) { summ += foo.x; });
    REQUIRE(summ == 6);
}

TEST_CASE_METHOD(WorldFixture, "View require", "[View]")
{
    world.Entity();

    Entity entt1 = world.Entity();
    entt1.Edit().Add<Foo>(1).Add<Bar>(1).Apply();

    int summ = 0;
    const auto view = world.View().Require<Foo, Bar>();
    view.ForEach([&](Foo foo, Bar bar) {
        summ += foo.x + bar.x;
        REQUIRE(foo.x == bar.x);
    });

    REQUIRE(summ == 2);

    summ = 0;

    Entity entt2 = world.Entity();
    entt2.Edit().Add<Foo>(2).Apply();
    entt2.Edit().Add<Bar>(2).Apply();

    view.ForEach([&](Foo foo, Bar bar) {
        summ += foo.x + bar.x;
        REQUIRE(foo.x == bar.x);
    });

    REQUIRE(summ == 6);

    world.Entity().Edit().Add<Foo>(999).Apply();
    summ = 0;

    view.ForEach([&](Foo foo) { summ += foo.x; });

    REQUIRE(summ == 3);
}

TEST_CASE_METHOD(WorldFixture, "View exclude", "[View]")
{
    world.Entity();

    world.Entity().Edit().Add<Foo>(1).Add<Bar>(1).Apply();
    world.Entity().Edit().Add<Foo>(1).Apply();

    int summ = 0;
    const auto view = world.View().Require<Foo>().Exclude<Bar>();
    view.ForEach([&](Foo foo) { summ += foo.x; });

    REQUIRE(summ == 1);
}

TEST_CASE_METHOD(WorldFixture, "View query for entity", "[View]")
{
    const auto entt1 = world.Entity().Edit().Add<int>(1).Apply();
    const auto entt2 = world.Entity().Edit().Add<int>(2).Apply();
    const auto entt3 = world.Entity().Edit().Add<int>(3).Apply();

    const auto view = world.View().Require<int>();
    int calls = 0;
    view.ForEntity(entt3, [&](int i) { calls++; REQUIRE(i == 3); });
    view.ForEntity(entt1.GetId(), [&](int i) { calls++; REQUIRE(i == 1); });
    view.ForEntity(entt2, [&](int i) { calls++; REQUIRE(i == 2); });

    REQUIRE(calls == 3);
}

TEST_CASE("View special args", "[View]")
{
    World world;
    const auto entt1 = world.Entity().Edit().Add<Foo>(1).Apply();

    const auto view = world.View().Require<Foo>();
    int calls = 0;
    view.ForEach([&](Foo, World& worldArg) { calls++; REQUIRE(&world == &worldArg); });
    view.ForEach([&](Foo, World* worldArg) { calls++; REQUIRE(&world == worldArg); });
    view.ForEntity(entt1, [&](Foo, World* worldArg) { calls++; REQUIRE(&world == worldArg); });
    REQUIRE(calls == 3);
}

TEST_CASE_METHOD(WorldFixture, "Query", "[Query]")
{
    world.Entity();

    Entity entt1 = world.Entity();
    entt1.Edit().Add<Foo>(1).Apply();

    int summ = 0;
    const auto query = world.Query().Require<Foo>().Build();
    query.ForEach([&](EntityId id, Foo foo) {
        summ += foo.x;
        REQUIRE(id == entt1.GetId());
    });

    REQUIRE(summ == 1);
    summ = 0;

    Entity entt2 = world.Entity();
    entt2.Edit().Add<Foo>(2).Add<Bar>(2).Apply();
    query.ForEach([&](Foo foo) {
        summ += foo.x;
    });

    REQUIRE(summ == 3);
}

TEST_CASE_METHOD(WorldFixture, "Query Exclude", "[Query]")
{
    Entity entt1 = world.Entity().Edit().Add<Foo>(1).Apply();
    world.Entity().Edit().Add<Foo>(1).Add<Bar>(1).Apply();

    int summ = 0;
    const auto query = world.Query().Require<Foo>().Exclude<Bar>().Build();
    query.ForEach([&](Foo foo) { summ += foo.x; });

    REQUIRE(summ == 1);
    summ = 0;

    Entity entt2 = world.Entity();
    entt2.Edit().Add<Foo>(2).Add<Bar>(2).Add<float>(0.0f).Apply();
    query.ForEach([&](EntityId id, Foo foo) {
        summ += foo.x;
        REQUIRE(id.rawId == entt1.GetId());
    });

    REQUIRE(summ == 1);
}