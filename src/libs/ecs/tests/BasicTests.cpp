#include <catch2/catch_test_macros.hpp>
#include <ecs/Ecs.hpp>

using namespace RR::Ecs;

TEST_CASE("Create entity", "[Entity]")
{
    SECTION("Create")
    {
        World world;
        Entity entt1 = world.Entity();
        Entity entt2 = world.Entity();
        Entity entt3 = world.Entity();
        REQUIRE(entt1.GetId().rawId == 0);
        REQUIRE(entt2.GetId().rawId == 1);
        REQUIRE(entt3.GetId().rawId == 2);
    }

    SECTION("Delete")
    {
        World world;
        Entity entt1 = world.Entity();
        Entity entt2 = world.Entity();
        REQUIRE(entt2.IsAlive());
        entt2.Destruct();
        REQUIRE(!entt2.IsAlive());
        Entity entt3 = world.Entity();
        REQUIRE(entt1.IsAlive());
        REQUIRE(entt3.IsAlive());
        REQUIRE(entt1.GetId().rawId == 0);
        REQUIRE(entt3.GetId().GetGeneration() ==  entt2.GetId().GetGeneration() + 1);
        REQUIRE(entt3.GetId().GetIndex() == entt2.GetId().GetIndex());
    }

    SECTION("Delete deleted")
    {
        World world;
        Entity entt1 = world.Entity();
        Entity entt2 = world.Entity();
        REQUIRE(entt2.IsAlive());
        entt2.Destruct();
        world.Destruct(entt2.GetId());
        Entity entt3 = world.Entity();
        REQUIRE(!entt2.IsAlive());
        REQUIRE(entt1.IsAlive());
        REQUIRE(entt3.IsAlive());
        REQUIRE(entt1.GetId().rawId == 0);
        REQUIRE(entt3.GetId().GetGeneration() == entt2.GetId().GetGeneration() + 1);
        REQUIRE(entt3.GetId().GetIndex() == entt2.GetId().GetIndex());
    }

    SECTION("Mutate")
    {
        // clang-format off
        struct Foo { int x;};
        struct Bar { int x;};
        // clang-format on
        World world;

        Entity entt1 = world.Entity();
        REQUIRE(!entt1.Has<Foo>());
        entt1.Mutate().Add<Foo>(1).Commit();
        REQUIRE(entt1.Has<Foo>());
        REQUIRE(!entt1.Has<Foo, Bar>());
        entt1.Mutate().Add<Bar>(1).Commit();
        REQUIRE(entt1.Has<Foo, Bar>());

        Entity entt2 = world.Entity();
        REQUIRE(!entt2.Has<Foo, Bar>());
        entt2.Mutate().Add<Foo>(1).Add<Bar>(1).Commit();
        REQUIRE(entt2.Has<Foo, Bar>());
    }

    SECTION("Remove")
    {
        // clang-format off
        struct Foo { int x;};
        struct Bar { int x;};
        // clang-format on
        World world;

        Entity entt1 = world.Entity().Mutate().Add<Foo>(1).Commit();
        REQUIRE(entt1.Has<Foo>());
        entt1.Mutate().Remove<Foo>().Commit();
        REQUIRE(!entt1.Has<Foo>());

        Entity entt2 = world.Entity().Mutate().Add<Foo>(1).Commit();
        REQUIRE(entt2.Has<Foo>());
        entt2.Mutate().Remove<Foo>().Add<Bar>(1).Commit();
        REQUIRE(entt2.Has<Bar>());
        REQUIRE(!entt2.Has<Foo>());
    }

    SECTION("Query")
    {
        World world;

        // clang-format off
        struct Foo { int x;};
        struct Bar { int x;};
        // clang-format on

        world.Entity();

        Entity entt1 = world.Entity();
        entt1.Mutate().Add<Foo>(1).Add<Bar>(1).Commit();

        int summ = 0;
        Query query = world.Query<Foo,Bar>().Build();
        query.Each([&](Foo foo, Bar bar)
        {
            summ += foo.x + bar.x;
            REQUIRE(foo.x == bar.x);
        });

        REQUIRE(summ == 2);

        summ = 0;

        Entity entt2 = world.Entity();
        entt2.Mutate().Add<Foo>(2).Commit();
        entt2.Mutate().Add<Bar>(2).Commit();

        query.Each([&](Foo foo, Bar bar)
        {
            summ += foo.x + bar.x;
            REQUIRE(foo.x == bar.x);
        });

        REQUIRE(summ == 6);

        world.Entity().Mutate().Add<Foo>(999).Commit();
        summ = 0;

        query.Each([&](Foo foo) { summ += foo.x; });

        REQUIRE(summ == 3);
    }
}
