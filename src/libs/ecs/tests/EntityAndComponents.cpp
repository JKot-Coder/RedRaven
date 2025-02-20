#include <catch2/catch_test_macros.hpp>
#include <ecs/Ecs.hpp>

using namespace RR::Ecs;

struct WorldFixture
{
    World world;
};

TEST_CASE_METHOD(WorldFixture, "Create Entity", "[Entity]")
{
    Entity entt1 = world.Entity();
    Entity entt2 = world.Entity();
    Entity entt3 = world.Entity();
    REQUIRE(entt1.GetId().rawId == 0);
    REQUIRE(entt2.GetId().rawId == 1);
    REQUIRE(entt3.GetId().rawId == 2);
}

TEST_CASE_METHOD(WorldFixture, "Delete Entity", "[Entity]")
{
    Entity entt1 = world.Entity();
    Entity entt2 = world.Entity();
    REQUIRE(entt2.IsAlive());
    entt2.Destruct();
    REQUIRE(!entt2.IsAlive());
    Entity entt3 = world.Entity();
    REQUIRE(entt1.IsAlive());
    REQUIRE(entt3.IsAlive());
    REQUIRE(entt1.GetId().rawId == 0);
    REQUIRE(entt3.GetId().GetGeneration() == entt2.GetId().GetGeneration() + 1);
    REQUIRE(entt3.GetId().GetIndex() == entt2.GetId().GetIndex());
}

TEST_CASE_METHOD(WorldFixture, "Delete deleted Entity", "[Entity]")
{
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

TEST_CASE_METHOD(WorldFixture, "Add Components", "[Components]")
{
    // clang-format off
    struct Foo { int x; int z;};
    struct Bar { int x;};
    // clang-format on
    Entity entt1 = world.Entity();
    REQUIRE(!entt1.Has<Foo>());
    entt1.Edit().Add<Foo>(1,1).Apply();
    REQUIRE(entt1.Has<Foo>());
    REQUIRE(!entt1.Has<Foo, Bar>());
    entt1.Edit().Add<Bar>(1).Apply();
    REQUIRE(entt1.Has<Foo, Bar>());

    Entity entt2 = world.Entity();
    REQUIRE(!entt2.Has<Foo, Bar>());
    entt2.Edit().Add<Foo>(1,1).Add<Bar>(1).Apply();
    REQUIRE(entt2.Has<Foo, Bar>());
}

TEST_CASE_METHOD(WorldFixture, "Remove Components", "[Components]")
{
    // clang-format off
    struct Foo { int x;};
    struct Bar { int x;};
    // clang-format on

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

TEST_CASE_METHOD(WorldFixture, "NonTrivial Components", "[Components]")
{
    using Vector = eastl::vector<int32_t>;
    Entity entt1 = world.Entity().Edit().Add<Vector>(1).Apply();
    REQUIRE(entt1.Has<Vector>());
    entt1.Edit().Remove<Vector>().Apply();
    REQUIRE(!entt1.Has<Vector>());
}

TEST_CASE_METHOD(WorldFixture, "Tags", "[Components]")
{
    struct Tag{};

    world.Entity().Edit().Add<Tag>().Apply();
    Entity entt2 = world.Entity().Edit().Add<Tag>().Apply();
    world.Entity().Edit().Add<Tag>().Apply();

    REQUIRE(entt2.Has<Tag>());
    entt2.Edit().Remove<Tag>().Apply();
    REQUIRE(!entt2.Has<Tag>());
}

TEST_CASE("Remove and Move NonTrivial Components", "[Components]")
{
    World& world  = *new World();
    enum class Op
    {
        Construct,
        Destuct,
        MoveFrom,
        MoveTo,
        Copy,
    };

    static eastl::vector<eastl::pair<int32_t, Op>> OpLog;

    struct NonTrivial
    {
        NonTrivial(int32_t id) : id(id) { OpLog.emplace_back(id, Op::Construct); };
        ~NonTrivial() { OpLog.emplace_back(id, Op::Destuct); };
        NonTrivial(const NonTrivial& other) {
            OpLog.emplace_back(other.id, Op::Copy);
            id = other.id;
        }
        NonTrivial(NonTrivial&& other)
        {
            OpLog.emplace_back(other.id, Op::MoveFrom);
            OpLog.emplace_back(id, Op::MoveTo);
            eastl::swap(id, other.id);
        }

        int32_t id;
    };

    world.Entity().Edit().Add<NonTrivial>(1).Apply();
    Entity entt2 = world.Entity().Edit().Add<NonTrivial>(2).Apply();
    world.Entity().Edit().Add<NonTrivial>(3).Apply();

    entt2.Edit().Remove<NonTrivial>().Apply();

    // Check no any copy
    world.Query<NonTrivial>().Build().Each([&](NonTrivial&) { });

    Entity entt3 = world.Entity().Edit().Add<NonTrivial>(4).Add<Op>(Op::Construct).Apply();
    entt3.Edit().Remove<Op>().Apply();

    REQUIRE(OpLog.size() == 9);
    REQUIRE(OpLog[0] == eastl::pair {1, Op::Construct});
    REQUIRE(OpLog[1] == eastl::pair {2, Op::Construct});
    REQUIRE(OpLog[2] == eastl::pair {3, Op::Construct});
    REQUIRE(OpLog[3] == eastl::pair {3, Op::MoveFrom});
    REQUIRE(OpLog[4] == eastl::pair {2, Op::MoveTo});
    REQUIRE(OpLog[5] == eastl::pair {2, Op::Destuct});

    REQUIRE(OpLog[6] == eastl::pair {4, Op::Construct});
    REQUIRE(OpLog[7] == eastl::pair {4, Op::Copy});
    REQUIRE(OpLog[8] == eastl::pair {4, Op::Destuct});

    delete &world;

    REQUIRE(OpLog.size() == 12);
    REQUIRE(OpLog[9] == eastl::pair {1, Op::Destuct});
    REQUIRE(OpLog[10] == eastl::pair {3, Op::Destuct});
    REQUIRE(OpLog[11] == eastl::pair {4, Op::Destuct});
}

TEST_CASE_METHOD(WorldFixture, "Moving NonTrivial Components", "[Components]")
{
    using Vector = eastl::vector<int32_t>;
    world.Entity().Edit().Add<Vector>(1).Apply();
    Entity entt2 = world.Entity().Edit().Add<Vector>(2).Apply();
    world.Entity().Edit().Add<Vector>(3).Apply();

    entt2.Edit().Remove<Vector>().Apply();
    REQUIRE(!entt2.Has<Vector>());

    int32_t summ = 0;
    world.Query<Vector>().Build().Each([&](Vector vector) { summ += vector[0]; });
    REQUIRE(summ == 4);
}

TEST_CASE("Query tests", "[Query]")
{
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
        Query query = world.Query<Foo, Bar>().Build();
        query.Each([&](Foo foo, Bar bar) {
            summ += foo.x + bar.x;
            REQUIRE(foo.x == bar.x);
        });

        REQUIRE(summ == 2);

        summ = 0;

        Entity entt2 = world.Entity();
        entt2.Edit().Add<Foo>(2).Apply();
        entt2.Edit().Add<Bar>(2).Apply();

        query.Each([&](Foo foo, Bar bar) {
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