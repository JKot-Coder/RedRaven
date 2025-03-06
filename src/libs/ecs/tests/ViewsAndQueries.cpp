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

TEST_CASE_METHOD(WorldFixture, "View", "[View]")
{
    world.Entity();

    world.Entity().Edit().Add<Foo>(1).Apply();
    world.Entity().Edit().Add<Bar>(1).Apply();
    world.Entity().Edit().Add<Foo>(1).Apply();

    int summ = 0;
    const auto view = world.View().Require<Foo>();
    view.Each([&](Foo foo) { summ += foo.x; });

    REQUIRE(summ == 2);
}

TEST_CASE_METHOD(WorldFixture, "View require", "[View]")
{
    world.Entity();

    Entity entt1 = world.Entity();
    entt1.Edit().Add<Foo>(1).Add<Bar>(1).Apply();

    int summ = 0;
    const auto view = world.View().Require<Foo, Bar>();
    view.Each([&](Foo foo, Bar bar) {
        summ += foo.x + bar.x;
        REQUIRE(foo.x == bar.x);
    });

    REQUIRE(summ == 2);

    summ = 0;

    Entity entt2 = world.Entity();
    entt2.Edit().Add<Foo>(2).Apply();
    entt2.Edit().Add<Bar>(2).Apply();

    view.Each([&](Foo foo, Bar bar) {
        summ += foo.x + bar.x;
        REQUIRE(foo.x == bar.x);
    });

    REQUIRE(summ == 6);

    world.Entity().Edit().Add<Foo>(999).Apply();
    summ = 0;

    view.Each([&](Foo foo) { summ += foo.x; });

    REQUIRE(summ == 3);
}

TEST_CASE_METHOD(WorldFixture, "View exclude", "[View]")
{
    world.Entity();

    world.Entity().Edit().Add<Foo>(1).Add<Bar>(1).Apply();
    world.Entity().Edit().Add<Foo>(1).Apply();

    int summ = 0;
    const auto view = world.View().Require<Foo>().Exclude<Bar>();
    view.Each([&](Foo foo) { summ += foo.x; });

    REQUIRE(summ == 1);
}

TEST_CASE("View special args", "[View]")
{
    World world;
    world.Entity().Edit().Add<Foo>(1).Apply();

    const auto view = world.View().Require<Foo>();
    int calls = 0;
    view.Each([&](Foo, World& worldArg) { calls++; REQUIRE(&world == &worldArg); });
    view.Each([&](Foo, World* worldArg) { calls++; REQUIRE(&world == worldArg); });
    REQUIRE(calls == 2);
}

TEST_CASE_METHOD(WorldFixture, "Query", "[Query]")
{
    world.Entity();

    Entity entt1 = world.Entity();
    entt1.Edit().Add<Foo>(1).Apply();

    int summ = 0;
    const auto query = world.Query().Require<Foo>().Build();
    query.Each([&](EntityId id, Foo foo) {
        summ += foo.x;
        REQUIRE(id.rawId == 1);
    });

    REQUIRE(summ == 1);
    summ = 0;

    Entity entt2 = world.Entity();
    entt2.Edit().Add<Foo>(2).Add<Bar>(2).Apply();
    query.Each([&](Foo foo) {
        summ += foo.x;
    });

    REQUIRE(summ == 3);
}

TEST_CASE_METHOD(WorldFixture, "Query Exclude", "[Query]")
{
    world.Entity().Edit().Add<Foo>(1).Apply();
    world.Entity().Edit().Add<Foo>(1).Add<Bar>(1).Apply();

    int summ = 0;
    const auto query = world.Query().Require<Foo>().Exclude<Bar>().Build();
    query.Each([&](Foo foo) { summ += foo.x; });

    REQUIRE(summ == 1);
    summ = 0;

    Entity entt2 = world.Entity();
    entt2.Edit().Add<Foo>(2).Add<Bar>(2).Add<float>(0.0f).Apply();
    query.Each([&](EntityId id, Foo foo) {
        summ += foo.x;
        REQUIRE(id.rawId == 0);
    });

    REQUIRE(summ == 1);
}