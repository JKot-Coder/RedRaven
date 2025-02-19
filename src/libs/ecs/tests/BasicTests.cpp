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

    SECTION("Edit")
    {
        // clang-format off
        struct Foo { int x;};
        struct Bar { int x;};
        // clang-format on
        World world;

        Entity entt1 = world.Entity();
        REQUIRE(!entt1.Has<Foo>());
        entt1.Edit().Add<Foo>(1).Apply();
        REQUIRE(entt1.Has<Foo>());
        REQUIRE(!entt1.Has<Foo, Bar>());
        entt1.Edit().Add<Bar>(1).Apply();
        REQUIRE(entt1.Has<Foo, Bar>());

        Entity entt2 = world.Entity();
        REQUIRE(!entt2.Has<Foo, Bar>());
        entt2.Edit().Add<Foo>(1).Add<Bar>(1).Apply();
        REQUIRE(entt2.Has<Foo, Bar>());
    }

    SECTION("Remove")
    {
        // clang-format off
        struct Foo { int x;};
        struct Bar { int x;};
        // clang-format on
        World world;

        Entity entt1 = world.Entity().Edit().Add<Foo>(1).Apply();
        REQUIRE(entt1.Has<Foo>());
        entt1.Edit().Remove<Foo>().Apply();
        REQUIRE(!entt1.Has<Foo>());

        Entity entt2 = world.Entity().Edit().Add<Foo>(1).Apply();
        REQUIRE(entt2.Has<Foo>());
        entt2.Edit().Remove<Foo>().Add<Bar>(1).Apply();
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
        entt1.Edit().Add<Foo>(1).Add<Bar>(1).Apply();

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
        entt2.Edit().Add<Foo>(2).Apply();
        entt2.Edit().Add<Bar>(2).Apply();

        query.Each([&](Foo foo, Bar bar)
        {
            summ += foo.x + bar.x;
            REQUIRE(foo.x == bar.x);
        });

        REQUIRE(summ == 6);

        world.Entity().Edit().Add<Foo>(999).Apply();
        summ = 0;

        query.Each([&](Foo foo) { summ += foo.x; });

        REQUIRE(summ == 3);
    }
}
