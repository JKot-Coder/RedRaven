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
                       .Property("x", &Foo::x)
                       .Property("y", &Foo::y)
                       .Property("z", &Foo::z)
                       .Info();

    Foo foo = {1241, 244, 3};

    Meta::Any any =  fooInfo.Get(&foo);
    REQUIRE(any.Cast<Foo>().x == 1241);
    REQUIRE(any.Cast<Foo>().y == 244);
    REQUIRE(any.Cast<Foo>().z == 3);

    REQUIRE(any.Properties().size() == 3);
    eastl::vector<int> results;
    for (auto property : any.Properties())
    {
        results.push_back(property.Cast<int>());
    }

    REQUIRE(results.size() == 3);
    REQUIRE(results[0] == 1241);
    REQUIRE(results[1] == 244);
    REQUIRE(results[2] == 3);
}
