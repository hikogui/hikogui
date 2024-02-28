// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "bigint.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(bigint) {

static_assert(std::numeric_limits<hi::ubig128>::min() == 0);
static_assert(std::numeric_limits<hi::ubig128>::max() > 0);
static_assert(std::numeric_limits<hi::big128>::min() < 0);
static_assert(std::numeric_limits<hi::big128>::max() > 0);

static_assert(0 == std::numeric_limits<hi::ubig128>::min());
static_assert(0 < std::numeric_limits<hi::ubig128>::max());
static_assert(0 > std::numeric_limits<hi::big128>::min());
static_assert(0 < std::numeric_limits<hi::big128>::max());

TEST_CASE(construct)
{
    REQUIRE(hi::ubig128("1").string() == "1");
    REQUIRE(hi::ubig128("10").string() == "10");
    REQUIRE(hi::ubig128("100").string() == "100");
    REQUIRE(hi::ubig128("1000").string() == "1000");
    REQUIRE(hi::ubig128("10000").string() == "10000");
    REQUIRE(hi::ubig128("100000").string() == "100000");
    REQUIRE(hi::ubig128("1000000").string() == "1000000");
    REQUIRE(hi::ubig128("10000000").string() == "10000000");
    REQUIRE(hi::ubig128("100000000").string() == "100000000");
    REQUIRE(hi::ubig128("1000000000").string() == "1000000000");
    REQUIRE(hi::ubig128("10000000000").string() == "10000000000");
    REQUIRE(hi::ubig128("100000000000").string() == "100000000000");
    REQUIRE(hi::ubig128("1000000000000").string() == "1000000000000");
    REQUIRE(hi::ubig128("10000000000000").string() == "10000000000000");
    REQUIRE(hi::ubig128("100000000000000").string() == "100000000000000");
    REQUIRE(hi::ubig128("1000000000000000").string() == "1000000000000000");
    REQUIRE(hi::ubig128("10000000000000000").string() == "10000000000000000");
    REQUIRE(hi::ubig128("100000000000000000").string() == "100000000000000000");
    REQUIRE(hi::ubig128("1000000000000000000").string() == "1000000000000000000");
    REQUIRE(hi::ubig128("10000000000000000000").string() == "10000000000000000000");
    REQUIRE(hi::ubig128("100000000000000000000").string() == "100000000000000000000");
    REQUIRE(hi::ubig128("1000000000000000000000").string() == "1000000000000000000000");
    REQUIRE(hi::ubig128("10000000000000000000000").string() == "10000000000000000000000");
    REQUIRE(hi::ubig128("100000000000000000000000").string() == "100000000000000000000000");
    REQUIRE(hi::ubig128("1000000000000000000000000").string() == "1000000000000000000000000");
    REQUIRE(hi::ubig128("10000000000000000000000000").string() == "10000000000000000000000000");

    REQUIRE(hi::ubig128("12").string() == "12");
    REQUIRE(hi::ubig128("123").string() == "123");
    REQUIRE(hi::ubig128("1234").string() == "1234");
    REQUIRE(hi::ubig128("12345").string() == "12345");
    REQUIRE(hi::ubig128("123456").string() == "123456");
    REQUIRE(hi::ubig128("1234567").string() == "1234567");
    REQUIRE(hi::ubig128("12345678").string() == "12345678");
    REQUIRE(hi::ubig128("123456789").string() == "123456789");
    REQUIRE(hi::ubig128("1234567890").string() == "1234567890");
    REQUIRE(hi::ubig128("12345678901").string() == "12345678901");
    REQUIRE(hi::ubig128("123456789012").string() == "123456789012");
    REQUIRE(hi::ubig128("1234567890123").string() == "1234567890123");
    REQUIRE(hi::ubig128("12345678901234").string() == "12345678901234");
    REQUIRE(hi::ubig128("123456789012345").string() == "123456789012345");
    REQUIRE(hi::ubig128("1234567890123456").string() == "1234567890123456");
    REQUIRE(hi::ubig128("12345678901234567").string() == "12345678901234567");
    REQUIRE(hi::ubig128("123456789012345678").string() == "123456789012345678");
    REQUIRE(hi::ubig128("1234567890123456789").string() == "1234567890123456789");
    REQUIRE(hi::ubig128("12345678901234567890").string() == "12345678901234567890");
    REQUIRE(hi::ubig128("123456789012345678901").string() == "123456789012345678901");
    REQUIRE(hi::ubig128("1234567890123456789012").string() == "1234567890123456789012");
    REQUIRE(hi::ubig128("12345678901234567890123").string() == "12345678901234567890123");
    REQUIRE(hi::ubig128("123456789012345678901234").string() == "123456789012345678901234");
    REQUIRE(hi::ubig128("1234567890123456789012345").string() == "1234567890123456789012345");
    REQUIRE(hi::ubig128("12345678901234567890123456").string() == "12345678901234567890123456");
    REQUIRE(hi::ubig128("123456789012345678901234567").string() == "123456789012345678901234567");
    REQUIRE(hi::ubig128("1234567890123456789012345678").string() == "1234567890123456789012345678");
    REQUIRE(hi::ubig128("12345678901234567890123456789").string() == "12345678901234567890123456789");
    REQUIRE(hi::ubig128("123456789012345678901234567890").string() == "123456789012345678901234567890");
}

TEST_CASE(shift_right)
{
    auto t = hi::ubig128{"10000000000000000000000000000"};

    REQUIRE(t == hi::ubig128{"10000000000000000000000000000"});
    t >>= 1;
    REQUIRE(t == hi::ubig128{"5000000000000000000000000000"});
    t >>= 2;
    REQUIRE(t == hi::ubig128{"1250000000000000000000000000"});
    t >>= 3;
    REQUIRE(t == hi::ubig128{"156250000000000000000000000"});
    t >>= 4;
    REQUIRE(t == hi::ubig128{"9765625000000000000000000"});
    t >>= 5;
    REQUIRE(t == hi::ubig128{"305175781250000000000000"});
    t >>= 6;
    REQUIRE(t == hi::ubig128{"4768371582031250000000"});
    t >>= 7;
    REQUIRE(t == hi::ubig128{"37252902984619140625"});
    t >>= 8;
    REQUIRE(t == hi::ubig128{"145519152283668518"});
}

TEST_CASE(shift_left)
{
    auto t = hi::ubig128{0b100101};
    auto u = hi::ubig128{0b100000};

    REQUIRE((t << 111) == hi::ubig128{"96057491882894311127814182090571776"});
    REQUIRE((u << 111) == hi::ubig128{"83076749736557242056487941267521536"});

    t <<= 111;
    u <<= 111;
    REQUIRE(t == hi::ubig128{"96057491882894311127814182090571776"});
    REQUIRE(u == hi::ubig128{"83076749736557242056487941267521536"});

    auto v = hi::ubig128{0x1f2e3d4c5b6a7988};
    v <<= 5;
    REQUIRE(v == hi::ubig128{"71897621192479027456"});
}

TEST_CASE(subtract)
{
    auto t = hi::ubig128{"83076749736557242056487941267521536"};

    REQUIRE(t - 1 == hi::ubig128{"83076749736557242056487941267521535"});

    t -= 1;
    REQUIRE(t == hi::ubig128{"83076749736557242056487941267521535"});
}

TEST_CASE(less)
{
    REQUIRE((hi::ubig128{"1ffff"} < hi::ubig128{"2bd40"}));
}

TEST_CASE(less_or_equal)
{
    static_assert(std::numeric_limits<signed char>::min() <= hi::big128{0});
    static_assert(std::numeric_limits<signed short>::min() <= hi::big128{0});
    static_assert(std::numeric_limits<signed int>::min() <= hi::big128{0});
    static_assert(std::numeric_limits<signed long>::min() <= hi::big128{0});
    static_assert(std::numeric_limits<signed long long>::min() <= hi::big128{0});
    static_assert(std::numeric_limits<hi::big128>::min() <= hi::big128{0});

    static_assert(hi::big128{0} <= std::numeric_limits<signed char>::max());
    static_assert(hi::big128{0} <= std::numeric_limits<signed short>::max());
    static_assert(hi::big128{0} <= std::numeric_limits<signed int>::max());
    static_assert(hi::big128{0} <= std::numeric_limits<signed long>::max());
    static_assert(hi::big128{0} <= std::numeric_limits<signed long long>::max());
    static_assert(hi::big128{0} <= std::numeric_limits<hi::big128>::max());
}

TEST_CASE(greater_or_equal)
{
    REQUIRE((hi::ubig128{"10000000000000000000000000000"} >= 32));
    REQUIRE((hi::ubig128{"33"} >= 32));
    REQUIRE((hi::ubig128{"32"} >= 32));
    REQUIRE(not (hi::ubig128{"31"} >= 32));
    REQUIRE(not (hi::ubig128{"1"} >= 32));
    REQUIRE(not (hi::ubig128{"0"} >= 32));
}

TEST_CASE(xor_test)
{
    REQUIRE((hi::ubig128{"0"} ^ hi::ubig128{"2"}) == hi::ubig128{"2"});
    REQUIRE((hi::ubig128{"2"} ^ hi::ubig128{"0"}) == hi::ubig128{"2"});
    REQUIRE((hi::ubig128{"2"} ^ hi::ubig128{"2"}) == hi::ubig128{"0"});

    REQUIRE((hi::ubig128{"0"} ^ hi::ubig128{"36893488147419103232"}) == hi::ubig128{"36893488147419103232"});
    REQUIRE((hi::ubig128{"36893488147419103232"} ^ hi::ubig128{"0"}) == hi::ubig128{"36893488147419103232"});
    REQUIRE((hi::ubig128{"36893488147419103232"} ^ hi::ubig128{"36893488147419103232"}) == hi::ubig128{"0"});

    REQUIRE((hi::ubig128{"0"} ^ hi::ubig128{"36893488147419103234"}) == hi::ubig128{"36893488147419103234"});
    REQUIRE((hi::ubig128{"36893488147419103234"} ^ hi::ubig128{"0"}) == hi::ubig128{"36893488147419103234"});
    REQUIRE((hi::ubig128{"36893488147419103234"} ^ hi::ubig128{"36893488147419103234"}) == hi::ubig128{"0"});

    REQUIRE((hi::ubig128{"2"} ^ hi::ubig128{"36893488147419103232"}) == hi::ubig128{"36893488147419103234"});
    REQUIRE((hi::ubig128{"36893488147419103232"} ^ hi::ubig128{"2"}) == hi::ubig128{"36893488147419103234"});

    REQUIRE((hi::ubig128{"2"} ^ hi::ubig128{"36893488147419103234"}) == hi::ubig128{"36893488147419103232"});
    REQUIRE((hi::ubig128{"36893488147419103234"} ^ hi::ubig128{"2"}) == hi::ubig128{"36893488147419103232"});
}

TEST_CASE(divide)
{
    auto t = hi::ubig128{"3689348814741910323200"};

    auto const[quotient, remainder] = div(t, 93);

    REQUIRE(quotient == hi::ubig128{"39670417362816240034"});
    REQUIRE(remainder == 38);
}

TEST_CASE(reciprocal_test)
{
    auto t = hi::ubig128{10};
    auto r = reciprocal(static_cast<hi::bigint<uint64_t, 4, false>>(t));
    constexpr auto r2 = reciprocal(hi::bigint<uint64_t, 4, false>{10});

    REQUIRE(r.digits[3] == uint64_t{0x19999999'99999999});
    REQUIRE(r.digits[2] == uint64_t{0x99999999'99999999});
    REQUIRE(r.digits[1] == uint64_t{0x99999999'99999999});
    REQUIRE(r.digits[0] == uint64_t{0x99999999'99999999});
    REQUIRE(r == r2);
}

TEST_CASE(multiply)
{
    auto t = hi::ubig128{0x1f2e3d4c5b6a7988};

    REQUIRE(t * 93 == hi::ubig128{"208952461590642173544"});
    t *= 93;
    REQUIRE(t == hi::ubig128{"208952461590642173544"});

    REQUIRE(t + 25 == hi::ubig128{"208952461590642173569"});
    t += 25;
    REQUIRE(t == hi::ubig128{"208952461590642173569"});
}

TEST_CASE(simple)
{
    {
        auto t = hi::ubig128{1};
        auto u = hi::ubig128{1};
        REQUIRE(t == u);
    }

    {
        auto t = hi::ubig128{"1"};
        auto u = hi::ubig128{"1"};
        REQUIRE(t == u);
    }

    {
        auto t = hi::ubig128{"123456789012345678901234567890"};
        auto u = hi::ubig128{"123456789012345678901234567890"};
        REQUIRE(t == u);
    }

    {
        auto t = hi::ubig128{"25"};
        auto u = hi::ubig128{5};
        u *= 5;
        REQUIRE(t == u);
    }

    {
        auto t = hi::ubig128{"9223372036854775857"};
        auto u = hi::ubig128{9223372036854775807};
        u += 50;
        REQUIRE(t == u);
    }
}

};