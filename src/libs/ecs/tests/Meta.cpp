#include <catch2/catch_all.hpp>
#include <ecs/Ecs.hpp>
#include "TestHelpers.hpp"

using namespace RR::Ecs;

struct Foo
{
    int x;
    int y;
    int z;
};

TEST_CASE_METHOD(WorldFixture, "Trivial type", "[Meta]")
{
    int foo = 123;
    Meta::Any any = world.RegisterComponent<int>().Info().Get(&foo);
    REQUIRE(any.Cast<int>() == 123);
}

TEST_CASE_METHOD(WorldFixture, "Struct with properties", "[Meta]")
{
    auto fooInfo = world.RegisterComponent<Foo>()
                       .Element("x", &Foo::x)
                       .Element("y", &Foo::y)
                       .Element("z", &Foo::z)
                       .Info();

    Foo foo = {1241, 244, 3};

    Meta::Any any =  fooInfo.Get(&foo);
    REQUIRE(any.Cast<Foo>().x == 1241);
    REQUIRE(any.Cast<Foo>().y == 244);
    REQUIRE(any.Cast<Foo>().z == 3);

    REQUIRE(any.Elements().size() == 3);
    eastl::vector<int> results;
    for (auto property : any.Elements())
    {
        results.push_back(property.Cast<int>());
    }

    REQUIRE(results.size() == 3);
    REQUIRE(results[0] == 1241);
    REQUIRE(results[1] == 244);
    REQUIRE(results[2] == 3);
}

TEST_CASE_METHOD(WorldFixture, "Components", "[Meta]")
{
    world.RegisterComponent<Foo>()
    .Element("x", &Foo::x)
    .Element("y", &Foo::y)
    .Element("z", &Foo::z);

    eastl::vector<uint32_t> results;
    auto entity = world.Entity().Add<Foo>(123, 234, 345).Apply();

    world.View().With<Foo>().ForEach([&results](World& world, EntityId entityid) {
        Entity entity = world.GetEntity(entityid); // Todo optimize it
        for (auto element : entity.Elements())
        {
            if (element.GetComponentId() == Meta::GetComponentId<EntityId>)
            {
                results.push_back(element.Cast<EntityId>().GetRawId());
            }
            else if (element.GetComponentId() == Meta::GetComponentId<Foo>)
            {
                for (auto property : element.Elements())
                    results.push_back(property.Cast<int>());
            }
            else
                ASSERT_MSG(false, fmt::format("Unknown component id for type:{}", element.GetComponentName()));
        }
    });

    REQUIRE(results.size() == 4);
    REQUIRE(results[0] == entity.GetId().GetRawId());
    REQUIRE(results[1] == 123);
    REQUIRE(results[2] == 234);
    REQUIRE(results[3] == 345);
}