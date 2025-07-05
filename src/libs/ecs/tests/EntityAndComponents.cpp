#include <catch2/catch_all.hpp>
#include <ecs/Ecs.hpp>
#include <ecs/Archetype.hpp>

using namespace RR::Ecs;

struct WorldFixture
{
    World world;
};

namespace {
    Archetype& resolveArchetype(Entity entt)
    {
        Archetype* archetype = nullptr;
        ArchetypeEntityIndex index;
        REQUIRE(entt.ResolveArhetype(archetype, index));
        REQUIRE(archetype);
        return *archetype;
    };

    template<typename Callable, typename CallableCheck>
    void immediateTest(Callable&& call, CallableCheck&& check)
    {
        World world;
        call(world);
        check(world);
    }

    template<typename Callable, typename CallableCheck>
    void defferedTest(Callable&& call, CallableCheck&& check)
    {
        World world;

        struct SingleExecutionToken{};
        world.Entity().Add<SingleExecutionToken>().Apply();

        bool called = false;
        world.View().With<SingleExecutionToken>().ForEach([&] {
            call(world);
            called = true;
        });

        check(world);

        REQUIRE(called);
    }
}

TEST_CASE("Create Entity", "[Entity]")
{
    auto test = [](World& world)
    {
        Entity entt1 = world.EmptyEntity();
        Entity entt2 = world.EmptyEntity();
        Entity entt3 = world.EmptyEntity();
        REQUIRE(entt1.GetId());
        REQUIRE(entt2.GetId().GetRawId() == entt1.GetId().GetRawId() + 1);
        REQUIRE(entt3.GetId().GetRawId() == entt2.GetId().GetRawId() + 1);
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Delete Entity", "[Entity]")
{
    auto test = [&](World& world) {
        Entity entt1 = world.EmptyEntity();
        Entity entt2 = world.EmptyEntity();
        REQUIRE(entt2.IsAlive());
        entt2.Destroy();
        REQUIRE(!entt2.IsAlive());
        Entity entt3 = world.EmptyEntity();
        REQUIRE(entt1.IsAlive());
        REQUIRE(entt3.IsAlive());
        REQUIRE(entt1.GetId());

        if (!world.IsLocked())
        {
            REQUIRE(entt3.GetId().GetGeneration() == entt2.GetId().GetGeneration() + 1);
            REQUIRE(entt3.GetId().GetIndex() == entt2.GetId().GetIndex());
        }
        else
        {
            REQUIRE(entt3.GetId().GetGeneration() == 0);
            REQUIRE(entt3.GetId().GetIndex() == entt2.GetId().GetIndex() + 1);
        }
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Delete deleted Entity", "[Entity]")
{
    auto test = [&](World& world) {
        Entity entt1 = world.EmptyEntity();
        Entity entt2 = world.EmptyEntity();
        REQUIRE(entt2.IsAlive());
        entt2.Destroy();
        world.Destroy(entt2.GetId());
        Entity entt3 = world.EmptyEntity();
        REQUIRE(!entt2.IsAlive());
        REQUIRE(entt1.IsAlive());
        REQUIRE(entt3.IsAlive());
        REQUIRE(entt1.GetId());
        if (!world.IsLocked())
        {
            REQUIRE(entt3.GetId().GetGeneration() == entt2.GetId().GetGeneration() + 1);
            REQUIRE(entt3.GetId().GetIndex() == entt2.GetId().GetIndex());
        }
        else
        {
            REQUIRE(entt3.GetId().GetGeneration() == 0);
            REQUIRE(entt3.GetId().GetIndex() == entt2.GetId().GetIndex() + 1);
        }
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Modify deleted Entity", "[Entity]")
{
    auto test = [&](World& world) {
        Entity entt1 = world.EmptyEntity();
        REQUIRE(entt1.IsAlive());
        entt1.Destroy();
        REQUIRE(!entt1.IsAlive());

        entt1.Edit().Add<float>(0.0f).Apply();
        REQUIRE(!entt1.Has<float>());
        entt1.Edit().Remove<float>().Apply();
        REQUIRE(!entt1.IsAlive());
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Add no components", "[Components]")
{
    auto test = [&](World& world) {
        REQUIRE(world.Entity().Apply().IsAlive());
        REQUIRE(world.EmptyEntity().Edit().Apply().IsAlive());
    };


    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Add Components", "[Components]")
{
    // clang-format off
    struct Foo { int x; int z; };
    struct Bar { int x; };
    // clang-format on

    auto test = [&](World& world) {
        Entity entt1 = world.EmptyEntity();
        REQUIRE(!entt1.Has<Foo>());
        entt1.Edit().Add<Foo>(1,1).Apply();
        REQUIRE(entt1.Has<Foo>());
        REQUIRE(!entt1.Has<Foo, Bar>());
        entt1.Edit().Add<Bar>(1).Apply();
        REQUIRE(entt1.Has<Foo, Bar>());

        Entity entt2 = world.EmptyEntity();
        REQUIRE(!entt2.Has<Foo, Bar>());
        entt2.Edit().Add<Foo>(1,1).Add<Bar>(1).Apply();
        REQUIRE(entt2.Has<Foo, Bar>());
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Remove Components", "[Components]")
{
    // clang-format off
    struct Foo { int x; };
    struct Bar { int x; };
    // clang-format on

    auto test = [&](World& world) {
        Entity entt1 = world.Entity().Add<Foo>(1).Apply();
        REQUIRE(entt1.Has<Foo>());
        entt1.Edit().Remove<Foo>().Apply();
        REQUIRE(!entt1.Has<Foo>());

        Entity entt2 = world.Entity().Add<Foo>(1).Apply();
        REQUIRE(entt2.Has<Foo>());
        entt2.Edit().Remove<Foo>().Add<Bar>(1).Apply();
        REQUIRE(entt2.Has<Bar>());
        REQUIRE(!entt2.Has<Foo>());
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Remove components on creation of entity", "[Entity]")
{
    auto test = [&](World& world) {
        REQUIRE_THROWS_WITH(world.Entity().Add<int>().Remove<int>().Apply(), "Can't remove components on creation of entity.");
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Remove non existing components", "[Entity]")
{
    auto test = [&](World& world) {
        world.Entity().Add<float>().Apply();
        Entity entt = world.Entity().Add<uint32_t>().Apply();
        REQUIRE_THROWS_WITH(entt.Edit().Remove<float>().Apply(), "Can't remove component float. Component is not present in the archetype.");
        REQUIRE_THROWS_WITH(entt.Edit().Remove<int>().Apply(), "Can't remove component <0X7C77B142>. Component is not present in the archetype.");
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Double add components", "[Entity]")
{
    auto test = [&](World& world) {
        REQUIRE_THROWS_WITH(world.Entity().Add<float>().Add<float>().Apply(), "Can't add component float. Only new components can be added.");
        Entity entt = world.Entity().Add<float>().Apply();
        REQUIRE_THROWS_WITH(entt.Edit().Add<float>().Apply(), "Can't add component float. Only new components can be added.");
        REQUIRE_THROWS_WITH(world.Entity().Add<EntityId>().Apply(), "Can't add component RR::Ecs::EntityId. Only new components can be added.");
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Add and remove components at the same time", "[Entity]")
{
    auto test = [&](World& world) {
        Entity entt = world.Entity().Add<float>().Apply();
        REQUIRE_THROWS_WITH(entt.Edit().Add<float>().Remove<float>().Apply(), "Can't add and remove components at the same time.");
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("NonTrivial Components", "[Components]")
{
    auto test = [&](World& world) {
        using Vector = eastl::vector<int32_t>;
        Entity entt1 = world.Entity().Add<Vector>(1).Apply();
        REQUIRE(entt1.Has<Vector>());
        entt1.Edit().Remove<Vector>().Apply();
        REQUIRE(!entt1.Has<Vector>());
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Tags", "[Components]")
{
    struct Tag{};
    struct Tag2{};

    auto test = [&](World& world) {
        world.Entity().Add<Tag>().Apply();
        Entity entt2 = world.Entity().Add<Tag>().Add<Tag2>().Apply();
        world.Entity().Add<Tag>().Apply();

        REQUIRE(entt2.Has<Tag>());
        REQUIRE(entt2.Has<Tag2>());
        entt2.Edit().Remove<Tag2>().Apply();
        REQUIRE(!entt2.Has<Tag2>());
        entt2.Edit().Remove<Tag>().Apply();
        REQUIRE(!entt2.Has<Tag>());
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE("Complex", "[Components]")
{
    struct Tag{};

    auto test = [&](World& world) {
        world.Entity().Add<int>(123).Add<Tag>().Add<float>(60.0f).Apply();
        Entity entt2 = world.Entity().Add<int>(124).Add<Tag>().Add<float>(70.0f).Apply();
        world.Entity().Add<int>(125).Add<Tag>().Add<float>(80.0f).Apply();

        entt2.Edit().Remove<Tag>().Apply();
    };

    auto check = [&](World& world) {
        eastl::vector<int> intResults;
        eastl::vector<float> floatResults;

        const auto view = world.View().With<int, float>();
        view.ForEach([&](int intValue, float floatValue) {
            intResults.push_back(intValue);
            floatResults.push_back(floatValue);
        });

        REQUIRE(intResults.size() == 3);
        REQUIRE(floatResults.size() == 3);

        REQUIRE(intResults[0] == 123);
        REQUIRE(intResults[1] == 125);
        REQUIRE(intResults[2] == 124);

        REQUIRE(floatResults[0] == 60.0f);
        REQUIRE(floatResults[1] == 80.0f);
        REQUIRE(floatResults[2] == 70.0f);
    };

    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE_METHOD(WorldFixture, "Tag size", "[Components]")
{
    struct Tag{};

    Entity entt2 = world.Entity().Add<Tag>().Apply();

    Archetype* archetype = nullptr;
    ArchetypeEntityIndex index;
    REQUIRE(entt2.ResolveArhetype(archetype, index));
    REQUIRE(archetype);
    REQUIRE(index.GetRaw() == 0);
    const auto componentIndex = archetype->GetComponentIndex(GetComponentId<Tag>);
    REQUIRE(componentIndex);
    REQUIRE(archetype->GetComponentInfo(componentIndex).size == 0);
}

TEST_CASE_METHOD(WorldFixture, "Heavy component", "[Comonents]")
{
    using HeavyComponent = std::array<uint32_t, 1024>;

    Entity heavy = world.Entity().Add<HeavyComponent>().Apply();
    Entity light = world.Entity().Add<int>().Apply();

    Archetype& heavyArch = resolveArchetype(heavy);
    Archetype& lightArch = resolveArchetype(light);

    REQUIRE(lightArch.GetChunkCapacity() > heavyArch.GetChunkCapacity() * 10);
    REQUIRE(heavyArch.GetEntitySize() == (sizeof(uint32_t) * 1024 + sizeof(EntityId)));
    REQUIRE(heavyArch.GetChunkSize() >= heavyArch.GetEntitySize() * heavyArch.GetChunkCapacity());

    std::vector<Entity> entities;
    for (size_t i = 0; i < heavyArch.GetChunkCapacity() * 3; i++)
        entities.push_back(world.Entity().Add<HeavyComponent>().Apply());

    for (auto entity : entities)
        entity.Edit().Remove<HeavyComponent>().Apply();
}

TEST_CASE_METHOD(WorldFixture, "Entities chunking", "[Comonents]")
{
    Entity entt = world.Entity().Add<int>().Apply();
    Archetype& intArch = resolveArchetype(entt);

    std::vector<Entity> entities;
    for (size_t i = 0; i < intArch.GetChunkCapacity() * 3; i++)
        entities.push_back(world.Entity().Add<int>().Apply());

    for (auto entity : entities)
        entity.Edit().Add<float>().Apply();

    for (auto entity : entities)
        entity.Edit().Remove<int>().Apply();

    REQUIRE(entities.front().Has<float>());
    REQUIRE(!entities.front().Has<int>());

    REQUIRE(entities.back().Has<float>());
    REQUIRE(!entities.back().Has<int>());
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
        ~NonTrivial()
        {
            OpLog.emplace_back(id, Op::Destuct);
            id = -1;
        };
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

    world.Entity().Add<NonTrivial>(1).Apply();
    Entity entt2 = world.Entity().Add<NonTrivial>(2).Apply();
    world.Entity().Add<NonTrivial>(3).Apply();

    entt2.Edit().Remove<NonTrivial>().Apply();

    // Check no any copy
    const auto view = world.View().With<NonTrivial>();
    view.ForEach([&](NonTrivial&) { });

    Entity entt3 = world.Entity().Add<NonTrivial>(4).Add<Op>(Op::Construct).Apply();
    entt3.Edit().Remove<Op>().Apply();

    size_t index = 0;
    REQUIRE(OpLog.size() == 10);
    REQUIRE(OpLog[index++] == eastl::pair {1, Op::Construct});
    REQUIRE(OpLog[index++] == eastl::pair {2, Op::Construct});
    REQUIRE(OpLog[index++] == eastl::pair {3, Op::Construct});
    REQUIRE(OpLog[index++] == eastl::pair {3, Op::MoveFrom});
    REQUIRE(OpLog[index++] == eastl::pair {2, Op::MoveTo});
    REQUIRE(OpLog[index++] == eastl::pair {2, Op::Destuct});

    REQUIRE(OpLog[index++] == eastl::pair {4, Op::Construct});
    REQUIRE(OpLog[index++] == eastl::pair {4, Op::MoveFrom});
    REQUIRE(OpLog[index++] == eastl::pair {-1, Op::MoveTo});
    REQUIRE(OpLog[index++] == eastl::pair {-1, Op::Destuct});

    delete &world;

    REQUIRE(OpLog.size() == 13);
    REQUIRE(OpLog[index++] == eastl::pair {1, Op::Destuct});
    REQUIRE(OpLog[index++] == eastl::pair {3, Op::Destuct});
    REQUIRE(OpLog[index++] == eastl::pair {4, Op::Destuct});
}

TEST_CASE("Moving NonTrivial Components", "[Components]")
{
    auto test = [&](World& world) {
        using Vector = eastl::vector<int32_t>;
        world.Entity().Add<Vector>(Vector{1}).Apply();
        Entity entt2 = world.Entity().Add<Vector>(Vector{2}).Apply();
        world.Entity().Add<Vector>(Vector{3}).Apply();

        entt2.Edit().Remove<Vector>().Apply();
        REQUIRE(!entt2.Has<Vector>());

        int32_t summ = 0;
        const auto view = world.View().With<Vector>();
        view.ForEach([&](Vector vector) { summ += vector[0]; });
        REQUIRE(summ == (world.IsLocked() ? 0 : 4));
    };

    auto check = [](World&) { };
    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

struct SingletonComponent {
    ECS_SINGLETON;
};

TEST_CASE("Singleton create", "[Singleton]")
{

    world.AddSingleton<SingletonComponent>();

    world.Entity().Add<int>().Apply();
    world.View().Singleton<SingletonComponent>().With<int>().ForEach([&]() { });
}
/*
#include <flecs.h>

TEST_CASE_METHOD(WorldFixture, "flect", "[Components]")
{
    flecs::world w;

    w.entity().set<int>(0);
    auto c = w.entity();
    w.defer_begin();
    w.each([&](int){
       //c.set<float>(0.f);
        c.destruct();
        REQUIRE(!c.is_alive());
    });
    w.defer_end();
    w.defer_end();
}*/