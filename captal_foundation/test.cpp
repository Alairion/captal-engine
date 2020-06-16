#include "src/config.hpp"
#include "src/stack_allocator.hpp"

#include <vector>

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_CONSOLE_WIDTH 120
#include "catch.hpp"
/*
TEST_CASE("Version check", "[version]")
{
    const cpt::version lowest{1, 4, 12};
    const cpt::version highest{1, 5, 2};

    SECTION("cpt::foundation::version is comparable")
    {
        REQUIRE(lowest == lowest);
        REQUIRE(highest == highest);
        REQUIRE(lowest != highest);
        REQUIRE(lowest <= lowest);
        REQUIRE(highest <= highest);
        REQUIRE(lowest >= lowest);
        REQUIRE(highest >= highest);
        REQUIRE(lowest < highest);
        REQUIRE(highest > lowest);
        REQUIRE(lowest <= highest);
        REQUIRE(highest >= lowest);
    }

    SECTION("cpt::foundation::version can be converted to and from a std::uint64_t representaion")
    {
        REQUIRE(cpt::foundation::unpack_version(cpt::foundation::pack_version(lowest)) == lowest);
        REQUIRE(cpt::foundation::unpack_version(cpt::foundation::pack_version(highest)) == highest);
    }
}

enum class test_enum : std::uint32_t
{
    none   = 0x00,
    first  = 0x04,
    second = 0x20,
    both   = first | second,
    third  = 0x4000,
    all    = first | second | third,
    others = ~all
};

template<>
struct cpt::enable_enum_operations<test_enum>{static constexpr bool value{true};};

TEST_CASE("Enum operations test", "[enums_ops]")
{
    using namespace cpt::enum_operations;

    SECTION("cpt::foundation support global '|', '&' and '~' operators for enum class types")
    {
        REQUIRE((test_enum::first | test_enum::second) == test_enum::both);
        REQUIRE((test_enum::both & test_enum::first) == test_enum::first);
        REQUIRE((test_enum::all & test_enum::both) == test_enum::both);
        REQUIRE((test_enum::first | test_enum::second | test_enum::third) == test_enum::all);
        REQUIRE((~test_enum::all) == test_enum::others);
        REQUIRE((test_enum::all & test_enum::none) == test_enum::none);
        REQUIRE((test_enum::both | test_enum::none) == test_enum::both);
        REQUIRE((~test_enum::none & test_enum::all) == test_enum::all);
    }

    SECTION("and also '|=' and '&=' operators")
    {
        test_enum value{test_enum::first};

        value |= test_enum::second;
        REQUIRE(value == test_enum::both);

        value &= ~test_enum::second;
        REQUIRE(value == test_enum::first);
    }
}

TEST_CASE("Stack allocator test", "[stack_alloc]")
{
    cpt::stack_memory_pool<4096> pool{};

    SECTION("cpt::stack_memory_pool can allocate up to stack_size - block_size bytes")
    {
        void* memory{pool.allocate(pool.stack_size - pool.block_size)};
        REQUIRE(memory);
        pool.deallocate(memory);
    }
}
*/

TEST_CASE("cpt::stack_vector vs std::vector")
{
    BENCHMARK("Small heap (~1Kio) stack_vector")
    {
        cpt::stack_memory_pool<1024> pool{};
        auto vector{cpt::make_stack_vector<float>(pool)};
        vector.resize(200);

        return vector.capacity();
    };

    BENCHMARK("Small heap (1Kio) vector")
    {
        std::vector<float> vector{};
        vector.resize(200);

        return vector.capacity();
    };

    BENCHMARK("Medium heap (8Kio) stack_vector")
    {
        cpt::stack_memory_pool<1024 * 8> pool{};
        auto vector{cpt::make_stack_vector<float>(pool)};
        vector.resize(2000);

        return vector.capacity();
    };

    BENCHMARK("Medium heap (8Kio) std::vector")
    {
        std::vector<float> vector{};
        vector.resize(2000);

        return vector.capacity();
    };

    BENCHMARK("Big heap (256Kio) stack_vector")
    {
        cpt::stack_memory_pool<1024 * 256> pool{};
        auto vector{cpt::make_stack_vector<float>(pool)};
        vector.resize(60000);

        return vector.capacity();
    };

    BENCHMARK("Big heap (256Kio) std::vector")
    {
        std::vector<float> vector{};
        vector.resize(60000);

        return vector.capacity();
    };

    BENCHMARK("Small heap overflow (1Kio) stack_vector")
    {
        cpt::stack_memory_pool<512> pool{};
        auto vector{cpt::make_stack_vector<float>(pool)};
        vector.resize(200);

        return vector.capacity();
    };

    BENCHMARK("Small heap overflow (1Kio) std::vector")
    {
        std::vector<float> vector{};
        vector.resize(200);

        return vector.capacity();
    };

    BENCHMARK("Big heap overflow (256Kio) stack_vector")
    {
        cpt::stack_memory_pool<1024 * 64> pool{};
        auto vector{cpt::make_stack_vector<float>(pool)};
        vector.resize(60000);

        return vector.capacity();
    };

    BENCHMARK("Big heap overflow (256Kio) std::vector")
    {
        std::vector<float> vector{};
        vector.resize(60000);

        return vector.capacity();
    };
}
