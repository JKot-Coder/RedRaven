#include <catch2/catch_test_macros.hpp>
#include <ecs/Ecs.hpp>
#include "TestHelpers.hpp"

using namespace RR::Ecs;

template <typename T>
struct TrackableType
{
    ECS_TRACKABLE;
    T x;
    bool operator==(const TrackableType& other) const
    {
        return x == other.x;
    }
};

using TrackableInt = TrackableType<int>;
using TrackableUInt = TrackableType<uint32_t>;

TEST_CASE_METHOD(WorldFixture, "Simple tracking", "[Tracking]")
{
    auto test = [&](World& world) {
        world.Entity().Add<TrackableInt>(1).Apply();
    };

    auto check = [&](World& world) {
        world.View().With<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
            trackableInt.x = 2;
        });

        world.View().With<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
            trackableInt.x = 3;
        });

        eastl::vector<int> results;
        world.System().Track<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
            results.push_back(trackableInt.x);
        });

        world.OrderSystems();
        world.ProcessTrackedChanges();
        REQUIRE(results.size() == 1);
        REQUIRE(results[0] == 3);
    };

    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE_METHOD(WorldFixture, "Delete tracked component", "[Tracking]")
{
    auto test = [&](World& world) {
        auto entity = world.Entity().Add<TrackableInt>(1).Apply();
        world.View().With<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
            trackableInt.x = 2;
        });

      entity.Edit().Remove<TrackableInt>().Apply();
    };

    auto check = [&](World& world) {
        eastl::vector<int> results;
        world.System().Track<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
            results.push_back(trackableInt.x);
        });

        world.OrderSystems();
        world.ProcessTrackedChanges();
        REQUIRE(results.size() == 0);
    };

    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE_METHOD(WorldFixture, "Move tracked components", "[Tracking]")
{
    struct DummyTag
    {
    };

    auto test = [&](World& world) {
        world.Entity().Add<TrackableInt>(1).Add<DummyTag>().Apply();
        auto entity2 = world.Entity().Add<TrackableInt>(2).Add<DummyTag>().Apply();
        world.Entity().Add<TrackableInt>(3).Add<DummyTag>().Apply();

        // We move entity2 to another archetype and entity3 to position of entity2
        entity2.Edit().Remove<DummyTag>().Apply();
    };

    auto check = [&](World& world) {
        eastl::vector<int> results;
        world.System().Track<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
            results.push_back(trackableInt.x);
        });
        world.OrderSystems();
        world.ProcessTrackedChanges(); // No changes

        world.View().With<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
            trackableInt.x++;
        });
        world.ProcessTrackedChanges();

        REQUIRE(results.size() == 3);
        REQUIRE(results[0] == 2);
        REQUIRE(results[1] == 4);
        REQUIRE(results[2] == 3);
    };

    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}

TEST_CASE_METHOD(WorldFixture, "Multiple tracked components", "[Tracking]")
{
    auto test = [&](World& world) {
        world.Entity().Add<TrackableInt>(1).Add<TrackableUInt>(1U).Apply();
        auto entity2 = world.Entity().Add<TrackableInt>(2).Add<TrackableUInt>(2U).Apply();
        world.Entity().Add<TrackableInt>(3).Add<TrackableUInt>(3U).Apply();
        world.Entity().Add<TrackableUInt>(4U).Apply();

        // We move entity2 to another archetype and entity3 to position of entity2
        entity2.Edit().Remove<TrackableUInt>().Apply();
    };

    auto check = [&](World& world) {
        eastl::vector<int> results;

        world.System().Track<TrackableInt>().Without<TrackableUInt>().ForEach([&](TrackableInt& trackableInt) {
            results.push_back(trackableInt.x);
        });

        world.System().Track<TrackableUInt>().Without<TrackableInt>().ForEach([&](TrackableUInt& trackableUInt) {
            results.push_back(trackableUInt.x);
        });

        world.System().Track<TrackableInt, TrackableUInt>().ForEach([&](TrackableInt& trackableInt, TrackableUInt& trackableUInt) {
            REQUIRE(trackableInt.x == int(trackableUInt.x));
            results.push_back(trackableInt.x);
        });
        world.OrderSystems();

        world.View().With<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
            trackableInt.x++;
        });

        world.View().With<TrackableUInt>().ForEach([&](TrackableUInt& trackableUInt) {
            trackableUInt.x++;
        });
        world.ProcessTrackedChanges();
        REQUIRE(results.size() == 4);
        REQUIRE(results[0] == 2);
        REQUIRE(results[1] == 4);
        REQUIRE(results[2] == 5);
        REQUIRE(results[3] == 3);
    };

    SECTION("Immediate") { immediateTest(test, check); }
    SECTION("Deffered") { defferedTest(test, check); }
}