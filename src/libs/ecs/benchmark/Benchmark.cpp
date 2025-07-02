#include <EASTL/bit.h>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <ecs/Ecs.hpp>
#include <flecs.h>
#include <nanobench.h>
#include <random>

using namespace RR::Ecs;

class random_xoshiro128
{
public:
    //***************************************************************************
    /// Default constructor.
    /// Attempts to come up with a unique non-zero seed.
    //***************************************************************************
    random_xoshiro128() noexcept
    {
        // An attempt to come up with a unique non-zero seed,
        // based on the address of the instance.
        const auto n = eastl::bit_cast<uintptr_t>(this);
        const auto seed = static_cast<uint32_t>(n);
        initialise(seed);
    }

    //***************************************************************************
    /// Constructor with seed value.
    ///\param seed The new seed value.
    //***************************************************************************
    explicit random_xoshiro128(uint32_t seed) noexcept { initialise(seed); }

    //***************************************************************************
    /// Initialises the sequence with a new seed value.
    ///\param seed The new seed value.
    //***************************************************************************
    constexpr void initialise(uint32_t seed) noexcept
    {
        // Add the first four primes to ensure that the seed isn't zero.
        state[0] = seed + 3;
        state[1] = seed + 5;
        state[2] = seed + 7;
        state[3] = seed + 11;
    }

    constexpr explicit random_xoshiro128(const std::array<uint32_t, 4>& s) : state(s) { }

    //***************************************************************************
    /// Get the next random_xoshiro128 number.
    //***************************************************************************
    constexpr uint32_t operator()() noexcept { return next(); }

    //***************************************************************************
    /// Get the next random_xoshiro128 number in a specified inclusive range.
    //***************************************************************************
    constexpr uint32_t range(uint32_t low, uint32_t high) noexcept
    {
        const uint32_t r = high - low + 1;
        return (operator()() % r) + low;
    }

    constexpr inline uint32_t range_max() noexcept
    {
        return range(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
    }

    constexpr inline uint32_t range_min(uint32_t min) noexcept
    {
        return range(min, std::numeric_limits<uint32_t>::max());
    }

public:
    std::array<uint32_t, 4> state;

private:
    /* This is xoshiro128** 1.1, one of our 32-bit all-purpose, rock-solid
       generators. It has excellent speed, a state size (128 bits) that is
       large enough for mild parallelism, and it passes all tests we are aware
       of.

       Note that version 1.0 had mistakenly s[0] instead of s[1] as state
       word passed to the scrambler.

       For generating just single-precision (i.e., 32-bit) floating-point
       numbers, xoshiro128+ is even faster.

       The state must be seeded so that it is not everywhere zero. */

    static inline constexpr uint32_t rotl(const uint32_t x, int k) noexcept { return (x << k) | (x >> (32 - k)); }

    constexpr uint32_t next() noexcept
    {
        const uint32_t result = rotl(state[1] * 5, 7) * 9;

        const uint32_t t = state[1] << 9;

        state[2] ^= state[0];
        state[3] ^= state[1];
        state[1] ^= state[2];
        state[0] ^= state[3];

        state[2] ^= t;

        state[3] = rotl(state[3], 11);

        return result;
    };
};

struct DataComponent
{
    inline static constexpr uint32_t DefaultSeed = 340383L;

    int thingy {0};
    double dingy {0.0};
    bool mingy {false};

    uint32_t seed {DefaultSeed};
    random_xoshiro128 rng;
    uint32_t numgy;

    DataComponent() : rng(seed), numgy(rng()) { }
};

struct VelocityComponent
{
    float x {1.0F};
    float y {1.0F};
};

struct PositionComponent
{
    float x {0.0F};
    float y {0.0F};
};

template <typename T>
struct EntityS
{
    T id;
    PositionComponent position;
    VelocityComponent velocity;
    DataComponent data;
};

TEST_CASE("Create archetype", "Archetype")
{
    ankerl::nanobench::Bench bench;
    bench.title("Create archetype")
        .warmup(100)
        .relative(true)
        .performanceCounters(true);
    bench.epochIterations(1);
    bench.relative(true);
    for (auto systemsCount : {100U, 1000U})
    {
        bench.epochs(systemsCount == 100 ? 60000: 3000);
        {
            bench.complexityN(4);
            bench.batch(4);
            bench.run("Ecs systems count:" + std::to_string(systemsCount), [&](ankerl::nanobench::Meter meter) {
                World world;
                for (uint32_t i = 0; i < systemsCount; i++)
                    world.System().OnEvent<Event>().ForEach([&]() { });

                world.OrderSystems();

                return meter.measure([&world]() {
                    world.Entity().Apply();
                    world.Entity().Add<int>(1).Apply();
                    world.Entity().Add<float>(1.0f).Apply();
                    world.Entity().Add<char>('c').Apply();
                });
                ankerl::nanobench::doNotOptimizeAway(&world);
            });
        }
        {
            bench.complexityN(16);
            bench.batch(16);
            bench.run("Ecs systems count:" + std::to_string(systemsCount), [&](ankerl::nanobench::Meter meter) {
                World world;
                for (uint32_t i = 0; i < systemsCount; i++)
                    world.System().OnEvent<Event>().ForEach([&]() { });

                world.OrderSystems();

                return meter.measure([&world]() {
                    world.Entity().Apply();
                    world.Entity().Add<int>(1).Apply();
                    world.Entity().Add<float>(1.0f).Apply();
                    world.Entity().Add<char>('c').Apply();

                    world.Entity().Add<double>(1.0).Apply();
                    world.Entity().Add<double>(1.0).Add<int>(1).Apply();
                    world.Entity().Add<double>(1.0).Add<float>(1.0f).Apply();
                    world.Entity().Add<double>(1.0).Add<char>('c').Apply();

                    world.Entity().Add<bool>(false).Apply();
                    world.Entity().Add<bool>(false).Add<int>(1).Apply();
                    world.Entity().Add<bool>(false).Add<float>(1.0f).Apply();
                    world.Entity().Add<bool>(false).Add<char>('c').Apply();

                    world.Entity().Add<uint16_t>(uint16_t(0)).Apply();
                    world.Entity().Add<uint16_t>(uint16_t(0)).Add<int>(1).Apply();
                    world.Entity().Add<uint16_t>(uint16_t(0)).Add<float>(1.0f).Apply();
                    world.Entity().Add<uint16_t>(uint16_t(0)).Add<char>('c').Apply();
                });
                ankerl::nanobench::doNotOptimizeAway(&world);
            });
        }
    }
}

TEST_CASE("Create Entity", "[Entity]")
{
    ankerl::nanobench::Bench bench;
    bench.title("Create Entity")
        .warmup(10)
        .relative(true)
        .performanceCounters(true);

    for (auto batchSize :
         {4U, 16U, 128U, 1024U, 10000U}) {

        bench.complexityN(batchSize);
        bench.batch(batchSize);
        bench.warmup(batchSize == 10000 ? 1 : 100);
        bench.epochIterations(1);
        bench.epochs(batchSize == 10000 ? 600 : 60000);
        bench.relative(true);

        {
            bench.run("AoS", [&](ankerl::nanobench::Meter meter) {
                eastl::vector<EntityS<uint64_t>> entities;

                uint64_t index = 0;
                return meter.measure([batchSize, &entities, &index]() {
                    for (uint32_t i = 0; i < batchSize; i++)
                        entities.push_back({index++, {1, 2}, {1, 2}, {}});
                });
                ankerl::nanobench::doNotOptimizeAway(&entities);
            });
        }

        {
            bench.run("SoA", [&](ankerl::nanobench::Meter meter) {
                uint64_t index = 0;
                eastl::vector<uint32_t> ids;
                eastl::vector<PositionComponent> positions;
                eastl::vector<VelocityComponent> velocities;
                eastl::vector<DataComponent> data;
                return meter.measure([&]() {

                    for (uint32_t i = 0; i < batchSize; i++)
                    {
                        ids.push_back(index++);
                        positions.push_back({1, 2});
                        velocities.push_back({1, 2});
                        data.push_back({});
                    }
                    ankerl::nanobench::doNotOptimizeAway(&ids);
                    ankerl::nanobench::doNotOptimizeAway(&positions);
                    ankerl::nanobench::doNotOptimizeAway(&velocities);
                    ankerl::nanobench::doNotOptimizeAway(&data);
                });
            });
        }

        auto polluteWorldWithArchetypes = [](World& world) {
            world.Entity().Add<float>(1.0f).Apply();
            world.Entity().Add<int>(1).Apply();
            world.Entity().Add<char>('c').Apply();
            world.Entity().Add<PositionComponent>(1.0f, 2.0f).Apply();
            world.Entity().Add<VelocityComponent>(1.0f, 2.0f).Apply();
            world.Entity().Add<DataComponent>().Apply();
            world.Entity().Add<float>(1.0f).Add<int>(1).Add<char>('c').Apply();
            world.Entity().Add<float>(1.0f).Add<int>(1).Apply();
        };

        {
            bench.run("Ecs", [&](ankerl::nanobench::Meter meter) {
                World world;
                polluteWorldWithArchetypes(world);

                return meter.measure([batchSize, &world]() {
                    for (uint32_t i = 0; i < batchSize; i++)
                        world.Entity().Add<PositionComponent>(1.0f, 2.0f).Add<VelocityComponent>(1.0f, 2.0f).Add<DataComponent>().Apply();
                });
                ankerl::nanobench::doNotOptimizeAway(&world);
            });
        }

        {
            bench.run("Ecs deffered", [&](ankerl::nanobench::Meter meter) {
                World world;
                polluteWorldWithArchetypes(world);

                struct OneShotExecuteToken{};

                world.Entity().Add<OneShotExecuteToken>().Apply();
                const auto query = world.Query().With<OneShotExecuteToken>().Build();

                return meter.measure([batchSize, &query]() {
                    query.ForEach([batchSize](World& world) {
                        for (uint32_t i = 0; i < batchSize; i++)
                            world.Entity().Add<PositionComponent>(1.0f, 2.0f).Add<VelocityComponent>(1.0f, 2.0f).Add<DataComponent>().Apply();
                    });
                });
                ankerl::nanobench::doNotOptimizeAway(&world);
            });
        }

        {
            bench.epochs(bench.epochs() / 10);
            bench.run("Flecs", [&](ankerl::nanobench::Meter meter) {
                flecs::world flecsWorld;

                return meter.measure([batchSize, &flecsWorld]() {
                   for (uint32_t i = 0; i < batchSize; i++)
                        flecsWorld.entity().set<PositionComponent>({1.0f, 2.0f}).set<VelocityComponent>({1.0f, 2.0f}).set<DataComponent>({});
                });
                ankerl::nanobench::doNotOptimizeAway(&flecsWorld);
            });
        }

        {
            bench.run("Chunks", [&](ankerl::nanobench::Meter meter) {
                eastl::vector<eastl::unique_ptr<std::byte[], eastl::default_delete<std::byte[]>>> chunks;
                uint32_t index = 0;
                return meter.measure([batchSize, &chunks, &index]() {
                    const size_t chunkSize = 16 * 1024;
                    const size_t chunkCapacity = 240;

                    for (uint32_t i = 0; i < batchSize; i++)
                    {
                        if (index >= chunkCapacity || chunks.empty())
                        {
                            chunks.push_back(eastl::make_unique<std::byte[]>(chunkSize));
                            index = 0;
                        }
                        eastl::array<size_t, 4> offsets = {0, 960, 2880, 4800};

                        uint32_t* __restrict entityIds = reinterpret_cast<uint32_t*>(chunks.back().get() + offsets[0]) + index;
                        PositionComponent* __restrict positions = reinterpret_cast<PositionComponent*>(chunks.back().get() + offsets[1]) + index;
                        VelocityComponent*__restrict velocities = reinterpret_cast<VelocityComponent*>(chunks.back().get() + offsets[2]) + index;
                        DataComponent* __restrict datas = reinterpret_cast<DataComponent*>(chunks.back().get() + offsets[3]) + index;

                        auto& __restrict entityId = *(entityIds);
                        auto& __restrict position = *(positions);
                        auto& __restrict velocity = *(velocities);
                        auto& __restrict data = *(datas);
                        entityId = index++;
                        position.x = 1;
                        position.y = 2;
                        velocity.x = 1;
                        velocity.y = 2;
                        data = {};
                    }
                    ankerl::nanobench::doNotOptimizeAway(&chunks);
                });
            });
        }
    }
}

TEST_CASE("Update Entity", "[Entity]")
{
    ankerl::nanobench::Bench bench;
    bench.title("Update entity")
        .warmup(1000)
        .relative(true)
        .performanceCounters(true);

    for (auto batchSize :
         {1U, 8U, 16U, 128U, 1024U, 100000U}) {

        const uint32_t numEntities = 10000;

        bench.complexityN(batchSize);
        bench.batch(batchSize);
        bench.minEpochTime(std::chrono::milliseconds(20));
       // bench.epochIterations(std::max(10000 - int(batchSize), 1));
        bench.epochs(100);
        bench.relative(true);

        {
            bench.run("AoS", [&](ankerl::nanobench::Meter meter) {
                eastl::vector<EntityS<uint64_t>> entities;

                uint64_t index = 0;
                for (uint32_t i = 0; i < numEntities; i++)
                    entities.push_back({index++, {1, 2}, {1, 2}, {}});

                std::random_device dev;
                ankerl::nanobench::Rng rng(dev());

                return meter.measure([&entities, batchSize, &rng]() {
                    for (uint32_t i = 0; i < batchSize; i++)
                    {
                        auto& entity = entities[rng() % numEntities];
                        entity.position.x += entity.velocity.x;
                        entity.position.y += entity.velocity.y;
                    }
                });
                ankerl::nanobench::doNotOptimizeAway(&entities);
            });
        }

        {
            bench.run("Ecs", [&](ankerl::nanobench::Meter meter) {
                eastl::vector<EntityId> entities;

                World world;
                for (uint32_t i = 0; i < numEntities; i++)
                    entities.push_back(
                        world
                            .Entity()
                            .Add<PositionComponent>(1.0f, 2.0f)
                            .Add<VelocityComponent>(1.0f, 2.0f)
                            .Add<DataComponent>()
                            .Apply()
                            .GetId());

                std::random_device dev;
                ankerl::nanobench::Rng rng(dev());
                const auto view = world.View().With<PositionComponent, VelocityComponent>();

                return meter.measure([&entities, batchSize, view, &rng]() {
                    for (uint32_t i = 0; i < batchSize; i++)
                        view.ForEntity(entities[rng() % numEntities], [&](PositionComponent& position, VelocityComponent& velocity) {
                            position.x += velocity.x;
                            position.y += velocity.y;
                        });
                });
                ankerl::nanobench::doNotOptimizeAway(&world);
            });
        }

        {
            bench.run("Flecs", [&](ankerl::nanobench::Meter meter) {
                flecs::world flecsWorld;
                eastl::vector<flecs::entity> entities;

                for (uint32_t i = 0; i < numEntities; i++)
                    entities.push_back(flecsWorld.entity().set<PositionComponent>({1.0f, 2.0f}).set<VelocityComponent>({1.0f, 2.0f}).set<DataComponent>({}));

                std::random_device dev;
                ankerl::nanobench::Rng rng(dev());

                return meter.measure([&entities, batchSize,  &rng]() {
                    for (uint32_t i = 0; i < batchSize; i++)
                    {
                        auto entity = (entities[rng() % numEntities]);
                        auto pos =  *entity.get<PositionComponent>();
                        pos.x += entity.get<VelocityComponent>()->x;
                        pos.y += entity.get<VelocityComponent>()->y;
                        entity.set(pos   );
                    }
                });
                ankerl::nanobench::doNotOptimizeAway(&flecsWorld);
            });
        }
    }
}

TEST_CASE("Update entities", "[Entity]")
{
    ankerl::nanobench::Bench bench;
    bench.title("Update entities")
        .warmup(1000)
        .relative(true)
        .performanceCounters(true);

    for (auto batchSize : {1U, 8U, 16U, 128U, 1024U, 100000U})
    {

        bench.complexityN(batchSize);
        bench.batch(batchSize);
        bench.minEpochTime(std::chrono::milliseconds(40));
        // bench.epochIterations(std::max(10000 - int(batchSize), 1));
        bench.epochs(100);
        bench.relative(true);

        {
            bench.run("SoA", [&](ankerl::nanobench::Meter meter) {
                eastl::vector<PositionComponent> positions;
                eastl::vector<VelocityComponent> velocities;

                for (uint32_t i = 0; i < batchSize; i++)
                {
                    positions.push_back({1, 2});
                    velocities.push_back({1, 2});
                }

                return meter.measure([&velocities, &positions, batchSize]() {
                    for (uint32_t i = 0; i < batchSize; i++)
                    {
                        const auto& velocity = velocities[i];
                        auto& position = positions[i];
                        position.x += velocity.x;
                        position.y += velocity.y;
                    }
                });
                ankerl::nanobench::doNotOptimizeAway(&positions);
            });
        }

        {
            bench.run("AoS", [&](ankerl::nanobench::Meter meter) {
                eastl::vector<EntityS<uint64_t>> entities;

                uint64_t index = 0;
                for (uint32_t i = 0; i < batchSize; i++)
                    entities.push_back({index++, {1, 2}, {1, 2}, {}});

                return meter.measure([&entities]() {
                    for (auto& entity : entities)
                    {
                        entity.position.x += entity.velocity.x;
                        entity.position.y += entity.velocity.y;
                    }
                });
                ankerl::nanobench::doNotOptimizeAway(&entities);
            });
        }
        {
            bench.run("Ecs query", [&](ankerl::nanobench::Meter meter) {
                World world;
                for (uint32_t i = 0; i < batchSize; i++)
                    world
                        .Entity()
                        .Add<PositionComponent>(1.0f, 2.0f)
                        .Add<VelocityComponent>(1.0f, 2.0f)
                        .Add<DataComponent>()
                        .Apply();

                const auto query = world.Query().With<PositionComponent, VelocityComponent>().Build();
                return meter.measure([query]() {
                    query.ForEach([&](PositionComponent& position, const VelocityComponent& velocity) {
                        position.x += velocity.x;
                        position.y += velocity.y;
                    });
                });
                ankerl::nanobench::doNotOptimizeAway(&world);
            });
        }
        {
            bench.run("Ecs system", [&](ankerl::nanobench::Meter meter) {
                World world;
                for (uint32_t i = 0; i < batchSize; i++)
                    world
                        .Entity()
                        .Add<PositionComponent>(1.0f, 2.0f)
                        .Add<VelocityComponent>(1.0f, 2.0f)
                        .Add<DataComponent>()
                        .Apply();

                struct BenchEvent : public Event { BenchEvent() : Event(GetEventId<BenchEvent>, sizeof(BenchEvent)) { } };
                world.System().With<PositionComponent, VelocityComponent>().OnEvent<BenchEvent>().ForEach([&](PositionComponent& position, const VelocityComponent& velocity) {
                    position.x += velocity.x;
                    position.y += velocity.y;
                });
                world.OrderSystems();

                return meter.measure([&world]() {
                    world.EmitImmediately<BenchEvent>({});
                });
                ankerl::nanobench::doNotOptimizeAway(&world);
            });
        }
        {
            bench.run("Flecs query", [&](ankerl::nanobench::Meter meter) {
                flecs::world flecsWorld;
                for (uint32_t i = 0; i < batchSize; i++)
                    flecsWorld.entity().set<PositionComponent>({1.0f, 2.0f}).set<VelocityComponent>({1.0f, 2.0f}).set<DataComponent>({});

                const auto query = flecsWorld.query<PositionComponent, VelocityComponent>();

                return meter.measure([query]() {
                    query.each([&](PositionComponent& position, VelocityComponent& velocity) {
                        position.x += velocity.x;
                        position.y += velocity.y;
                    });
                });
                ankerl::nanobench::doNotOptimizeAway(&flecsWorld);
            });
        }
        {
            bench.run("Flecs system", [&](ankerl::nanobench::Meter meter) {
                flecs::world world;
                for (uint32_t i = 0; i < batchSize; i++)
                    world.entity().set<PositionComponent>({1.0f, 2.0f}).set<VelocityComponent>({1.0f, 2.0f}).set<DataComponent>({});

                world.system<PositionComponent, VelocityComponent>().kind(flecs::OnUpdate).each([](PositionComponent& position, VelocityComponent& velocity) {
                    position.x += velocity.x;
                    position.y += velocity.y;
                });

                // warmup
                world.progress();
                return meter.measure([&world]() { world.progress(); });
                ankerl::nanobench::doNotOptimizeAway(&world);
            });
        }
        {
            bench.run("Chunks", [&](ankerl::nanobench::Meter meter) {
                const size_t chunkSize = 16 * 1024;
                const size_t chunkCapacity = 240;

                eastl::vector<std::byte*> chunks;
                eastl::array<size_t, 4> offsets = {0, 960, 2880, 4800};

                for (size_t i = 0; i < (batchSize / chunkCapacity) + 2; i++)
                    chunks.push_back(new std::byte[chunkSize]);

                uint32_t index = 0;
                for (size_t i = 0; i < chunks.size(); i++)
                {
                    uint32_t* entityIds = reinterpret_cast<uint32_t*>(chunks[i] + offsets[0]);
                    PositionComponent* positions = reinterpret_cast<PositionComponent*>(chunks[i] + offsets[1]);
                    VelocityComponent* velocities = reinterpret_cast<VelocityComponent*>(chunks[i] + offsets[2]);
                    DataComponent* datas = reinterpret_cast<DataComponent*>(chunks[i] + offsets[3]);

                    for (size_t j = 0; j < chunkCapacity && index < batchSize; j++, index++)
                    {
                        auto& entityId = *(entityIds++);
                        auto& position = *(positions++);
                        auto& velocity = *(velocities++);
                        auto& data = *(datas++);
                        entityId = index;
                        position.x = 1;
                        position.y = 2;
                        velocity.x = 1;
                        velocity.y = 2;
                        data = {};
                    }
                }

                return meter.measure([&chunks, batchSize, offsets]() {
                    uint32_t index = 0;
                    for (size_t i = 0; i < chunks.size()-1; i++)
                    {
                        PositionComponent* __restrict positions = reinterpret_cast<PositionComponent*>(chunks[i] + offsets[1]);
                        VelocityComponent*  __restrict velocities = reinterpret_cast<VelocityComponent*>(chunks[i] + offsets[2]);

#if defined(__i386__) || defined(__x86_64__)
                        _mm_prefetch(reinterpret_cast<const char*>(*chunks[i + 1]), _MM_HINT_T0);
#endif
                        for (size_t j = 0; j < chunkCapacity && index < batchSize; j++, index++)
                        {
                            auto& __restrict position = *(positions++);
                            auto& __restrict velocity = *(velocities++);
                            position.x += velocity.x;
                            position.y += velocity.y;
                        }
                    }
                });
            });
        }
    }
}

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
using TrackableFloat = TrackableType<float>;
using TrackableDouble = TrackableType<double>;

TEST_CASE("Tracking", "[Tracking]")
{
    ankerl::nanobench::Bench bench;
    bench.title("Tracking")
        .warmup(1000)
        .relative(true)
        .performanceCounters(true);

    for (auto batchSize : {128U, 1024U, 100000U})
    {
        bench.complexityN(batchSize);
        bench.batch(batchSize);
        bench.minEpochTime(std::chrono::milliseconds(100));
        bench.epochs(100);
        bench.relative(true);

        {
            bench.run("ECS", [&](ankerl::nanobench::Meter meter) {
                World world;
                for (uint32_t i = 0; i < batchSize; i++)
                    world.Entity().Add<TrackableInt>(1).Add<TrackableFloat>(1.0f).Add<TrackableDouble>(1.0).Apply();

                world.System().Track<TrackableInt>().ForEach([&](TrackableInt& trackableInt) {
                    trackableInt.x = 0;
                });
                world.System().Track<TrackableFloat>().ForEach([&](TrackableFloat& trackableFloat) {
                    trackableFloat.x = 0;
                });
                world.System().Track<TrackableDouble>().ForEach([&](TrackableDouble& trackableDouble) {
                    trackableDouble.x = 0;
                });

                world.View().With<TrackableInt, TrackableFloat, TrackableDouble>().ForEach([&](TrackableInt& trackableInt, TrackableFloat& trackableFloat, TrackableDouble& trackableDouble) {
                    trackableInt.x++;
                    trackableFloat.x++;
                    trackableDouble.x++;
                });

                world.OrderSystems();

                return meter.measure([&world]() {
                    world.ProcessTrackedChanges();
                });
                ankerl::nanobench::doNotOptimizeAway(&world);
            });
        }
    }
}

/*
TEST_CASE("hashmap", "[Entity]")
{
    ankerl::nanobench::Bench bench;
    bench.title("hasmap")
        .warmup(1000)
        .relative(true)
        .performanceCounters(true);

    for (auto batchSize :
         {1U, 8U, 16U, 128U, 1024U, 100000U}) {

        const uint32_t numEntities = 10000;

        bench.complexityN(batchSize);
        bench.batch(batchSize);
        bench.minEpochTime(std::chrono::milliseconds(20));
       // bench.epochIterations(std::max(10000 - int(batchSize), 1));
        bench.epochs(100);
        bench.relative(true);

        {
            bench.run("Ska", [&](ankerl::nanobench::Meter meter) {
                ska::flat_hash_map<uint64_t, uint64_t> map;
                eastl::vector<uint64_t> keys;

                std::random_device dev;
                ankerl::nanobench::Rng rng(dev());
                for (uint32_t i = 0; i < numEntities; i++)
                {
                    keys.push_back(rng());
                    map[keys.back()] = rng();
                }

                return meter.measure([&map, &rng, &keys, batchSize]() {
                    for (uint32_t i = 0; i < batchSize; i++)
                        map[keys[rng.bounded(numEntities - 1)]] = rng();
                });
                ankerl::nanobench::doNotOptimizeAway(&map);
            });
        }

        {
            bench.run("Absl", [&](ankerl::nanobench::Meter meter) {
                absl::flat_hash_map<uint64_t, uint64_t> map;
                eastl::vector<uint64_t> keys;

                std::random_device dev;
                ankerl::nanobench::Rng rng(dev());
                for (uint32_t i = 0; i < numEntities; i++)
                {
                    keys.push_back(rng());
                    map[keys.back()] = rng();
                }

                return meter.measure([&map, &rng, &keys, batchSize]() {
                    for (uint32_t i = 0; i < batchSize; i++)
                        map[keys[rng.bounded(numEntities - 1)]] = rng();
                });
                ankerl::nanobench::doNotOptimizeAway(&map);
            });
        }
    }
}*/