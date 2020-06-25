#include <captal_foundation/version.hpp>
#include <captal_foundation/encoding.hpp>
#include <captal_foundation/optional_ref.hpp>
#include <captal_foundation/enum_operations.hpp>
#include <captal_foundation/stack_allocator.hpp>

#include <vector>

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_CONSOLE_WIDTH 120
#include "catch.hpp"

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
    SECTION("cpt::stack_memory_pool is able to allocate up to 'pool.stack_size - pool.block_size' bytes with a unique allocation")
    {
        cpt::stack_memory_pool<512> pool{};

        void* memory{pool.allocate(pool.stack_size - pool.block_size)};
        REQUIRE(memory);
        pool.deallocate(memory);
    }

    SECTION("cpt::stack_memory_pool is able to allocate multiple memory pages")
    {
        cpt::stack_memory_pool<512> pool{};

        void* memory1{pool.allocate(24)};
        void* memory2{pool.allocate(24)};
        REQUIRE(memory1);
        REQUIRE(memory2);
        pool.deallocate(memory1);
        pool.deallocate(memory2);
    }

    SECTION("cpt::stack_memory_pool is able to reallocate freed memory pages if subsecant allocations' size are <= that a freed one")
    {
        cpt::stack_memory_pool<512> pool{};

        void* memory1{pool.allocate(24)};
        REQUIRE(memory1);
        pool.deallocate(memory1);

        void* memory2{pool.allocate(24)};
        REQUIRE(memory2);
        REQUIRE(memory1 == memory2);
        pool.deallocate(memory2);

        void* memory3{pool.allocate(8)};
        REQUIRE(memory3);
        REQUIRE(memory1 == memory3);
        pool.deallocate(memory3);
    }

    SECTION("cpt::stack_allocator is an Allocator that allocate memory within a stack_memory_pool")
    {
        cpt::stack_memory_pool<512> pool{};
        cpt::stack_allocator<std::uint32_t, 512> allocator{pool};

        std::uint32_t* memory{allocator.allocate(42)};
        REQUIRE(memory);
        allocator.deallocate(memory, 42);
    }

    SECTION("cpt::stack_allocator fallbacks on new/delete if the requested allocation fails")
    {
        cpt::stack_memory_pool<512> pool{};
        cpt::stack_allocator<std::uint32_t, 512> allocator{pool};

        std::uint32_t* memory{allocator.allocate(1000)};
        REQUIRE(memory);
        allocator.deallocate(memory, 1000);
    }
}

TEST_CASE("Encoding test", "[encoding]")
{
    const std::u8string_view string{u8"abcÀçè中国日本国кир👦"}; //A string with a lot of special chars with different sizes (in UTF-8)
    const std::size_t codepoint_count{15};

    REQUIRE(cpt::convert<cpt::wide, cpt::utf8>(
            cpt::convert<cpt::narrow, cpt::wide>(
            cpt::convert<cpt::utf32, cpt::narrow>(
            cpt::convert<cpt::utf16, cpt::utf32>(
            cpt::convert<cpt::utf8, cpt::utf16>(string))))) == string);

    const auto count = [string] <cpt::encoding Encoding> ()
    {
        const auto output{cpt::convert<cpt::utf8, Encoding>(string)};
        return Encoding::count(std::begin(output), std::end(output));
    };

    REQUIRE(count.operator()<cpt::utf8>() == codepoint_count);
    REQUIRE(count.operator()<cpt::utf16>() == codepoint_count);
    REQUIRE(count.operator()<cpt::utf32>() == codepoint_count);
    REQUIRE(count.operator()<cpt::narrow>() == codepoint_count);
    REQUIRE(count.operator()<cpt::wide>() == codepoint_count);
}

/*
static constexpr std::size_t pool_size{1024};

TEST_CASE("cpt::stack_allocator benchmark", "[stack_alloc_bench]")
{
    BENCHMARK("cpt::stack_vector fallbacks on new")
    {
        cpt::stack_memory_pool<pool_size> pool{};
        auto vector{cpt::make_stack_vector<std::uint32_t>(pool)};
        vector.resize(pool_size / 2);

        return vector.capacity();
    };

    BENCHMARK("cpt::stack_vector NO fallbacks on new")
    {
        cpt::stack_memory_pool<pool_size> pool{};
        auto vector{cpt::make_stack_vector<std::uint32_t>(pool)};
        vector.resize(pool_size / 8);

        return vector.capacity();
    };

    BENCHMARK("std::vector")
    {
        std::vector<std::uint32_t> vector{};
        vector.resize(pool_size / 2);

        return vector.capacity();
    };
}
*/