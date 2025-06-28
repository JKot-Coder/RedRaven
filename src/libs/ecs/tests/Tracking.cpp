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
using TrackableFloat = TrackableType<float>;

TEST_CASE_METHOD(WorldFixture, "Simple tracking", "[Tracking]")
{
    eastl::vector<int> results;

    world.Entity().Add<TrackableInt>(1).Apply();
    world.System().Track<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
        results.push_back(trackableInt.x);
    });

    world.View().With<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
        trackableInt.x = 2;
    });

    world.OrderSystems();
    world.ProcessTrackedChanges();

    REQUIRE(results.size() == 1);
    REQUIRE(results[0] == 2);
}

TEST_CASE_METHOD(WorldFixture, "Move tracked components", "[Tracking]")
{
    struct DummyTag {};
    eastl::vector<int> results;

    world.System().Track<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
        results.push_back(trackableInt.x);
    });
    world.OrderSystems();

    world.Entity().Add<TrackableInt>(1).Add<DummyTag>().Apply();
    auto entity2 = world.Entity().Add<TrackableInt>(2).Add<DummyTag>().Apply();
    world.Entity().Add<TrackableInt>(3).Add<DummyTag>().Apply();
    world.ProcessTrackedChanges(); // No changes

    world.View().With<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
        trackableInt.x++;
    });

    // We move entity2 to another archetype and entity3 to position of entity2
    entity2.Edit().Remove<DummyTag>().Apply();
    world.ProcessTrackedChanges();

    REQUIRE(results.size() == 3);
    REQUIRE(results[0] == 2);
    REQUIRE(results[1] == 4);
    REQUIRE(results[2] == 3);
}

TEST_CASE_METHOD(WorldFixture, "Deffered creation check", "[Tracking]")
{
    struct DummyTag {};
    eastl::vector<int> results;

    world.System().Track<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
        results.push_back(trackableInt.x);
    });
    world.OrderSystems();

    EntityId entity2;

    world.Entity().Add<DummyTag>().Apply();
    world.View().With<DummyTag>().ForEach([&entity2](World& world) {
        std::array<ComponentId, 2> componentIds = {GetComponentId<TrackableInt>, GetComponentId<DummyTag>};
        UNUSED(componentIds);
        world.Entity().Add<TrackableInt>(1).Add<DummyTag>().Apply();
        entity2 = world.Entity().Add<TrackableInt>(2).Add<DummyTag>().Apply().GetId();
        world.Entity().Add<TrackableInt>(3).Add<DummyTag>().Apply();
    });
    world.ProcessTrackedChanges(); // No changes

    world.View().With<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
        trackableInt.x++;
    });

    // We move entity2 to another archetype and entity3 to position of entity2
    world.GetEntity(entity2).Edit().Remove<DummyTag>().Apply();
    world.ProcessTrackedChanges();

    REQUIRE(results.size() == 3);
    REQUIRE(results[0] == 2);
    REQUIRE(results[1] == 4);
    REQUIRE(results[2] == 3);
}
