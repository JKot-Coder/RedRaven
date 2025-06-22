#include <catch2/catch_test_macros.hpp>
#include <ecs/Ecs.hpp>

using namespace RR::Ecs;

struct WorldFixture
{
    World world;
};

template <typename T>
struct TrackableType
{
    static constexpr bool Trackable = true;
    T x;
    bool operator==(const TrackableType& other) const
    {
        return x == other.x;
    }
};

using TrackableInt = TrackableType<int>;


TEST_CASE_METHOD(WorldFixture, "Simple create", "[Tracking]")
{
    world.Entity().Add<TrackableInt>(1).Apply();

}