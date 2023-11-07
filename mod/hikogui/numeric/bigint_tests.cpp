// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "bigint.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace std;
using namespace hi;

static_assert(std::numeric_limits<ubig128>::min() == 0);
static_assert(std::numeric_limits<ubig128>::max() > 0);
static_assert(std::numeric_limits<big128>::min() < 0);
static_assert(std::numeric_limits<big128>::max() > 0);

static_assert(0 == std::numeric_limits<ubig128>::min());
static_assert(0 < std::numeric_limits<ubig128>::max());
static_assert(0 > std::numeric_limits<big128>::min());
static_assert(0 < std::numeric_limits<big128>::max());

TEST(BigInt, Construct)
{
    ASSERT_EQ(ubig128("1").string(), "1");
    ASSERT_EQ(ubig128("10").string(), "10");
    ASSERT_EQ(ubig128("100").string(), "100");
    ASSERT_EQ(ubig128("1000").string(), "1000");
    ASSERT_EQ(ubig128("10000").string(), "10000");
    ASSERT_EQ(ubig128("100000").string(), "100000");
    ASSERT_EQ(ubig128("1000000").string(), "1000000");
    ASSERT_EQ(ubig128("10000000").string(), "10000000");
    ASSERT_EQ(ubig128("100000000").string(), "100000000");
    ASSERT_EQ(ubig128("1000000000").string(), "1000000000");
    ASSERT_EQ(ubig128("10000000000").string(), "10000000000");
    ASSERT_EQ(ubig128("100000000000").string(), "100000000000");
    ASSERT_EQ(ubig128("1000000000000").string(), "1000000000000");
    ASSERT_EQ(ubig128("10000000000000").string(), "10000000000000");
    ASSERT_EQ(ubig128("100000000000000").string(), "100000000000000");
    ASSERT_EQ(ubig128("1000000000000000").string(), "1000000000000000");
    ASSERT_EQ(ubig128("10000000000000000").string(), "10000000000000000");
    ASSERT_EQ(ubig128("100000000000000000").string(), "100000000000000000");
    ASSERT_EQ(ubig128("1000000000000000000").string(), "1000000000000000000");
    ASSERT_EQ(ubig128("10000000000000000000").string(), "10000000000000000000");
    ASSERT_EQ(ubig128("100000000000000000000").string(), "100000000000000000000");
    ASSERT_EQ(ubig128("1000000000000000000000").string(), "1000000000000000000000");
    ASSERT_EQ(ubig128("10000000000000000000000").string(), "10000000000000000000000");
    ASSERT_EQ(ubig128("100000000000000000000000").string(), "100000000000000000000000");
    ASSERT_EQ(ubig128("1000000000000000000000000").string(), "1000000000000000000000000");
    ASSERT_EQ(ubig128("10000000000000000000000000").string(), "10000000000000000000000000");

    ASSERT_EQ(ubig128("12").string(), "12");
    ASSERT_EQ(ubig128("123").string(), "123");
    ASSERT_EQ(ubig128("1234").string(), "1234");
    ASSERT_EQ(ubig128("12345").string(), "12345");
    ASSERT_EQ(ubig128("123456").string(), "123456");
    ASSERT_EQ(ubig128("1234567").string(), "1234567");
    ASSERT_EQ(ubig128("12345678").string(), "12345678");
    ASSERT_EQ(ubig128("123456789").string(), "123456789");
    ASSERT_EQ(ubig128("1234567890").string(), "1234567890");
    ASSERT_EQ(ubig128("12345678901").string(), "12345678901");
    ASSERT_EQ(ubig128("123456789012").string(), "123456789012");
    ASSERT_EQ(ubig128("1234567890123").string(), "1234567890123");
    ASSERT_EQ(ubig128("12345678901234").string(), "12345678901234");
    ASSERT_EQ(ubig128("123456789012345").string(), "123456789012345");
    ASSERT_EQ(ubig128("1234567890123456").string(), "1234567890123456");
    ASSERT_EQ(ubig128("12345678901234567").string(), "12345678901234567");
    ASSERT_EQ(ubig128("123456789012345678").string(), "123456789012345678");
    ASSERT_EQ(ubig128("1234567890123456789").string(), "1234567890123456789");
    ASSERT_EQ(ubig128("12345678901234567890").string(), "12345678901234567890");
    ASSERT_EQ(ubig128("123456789012345678901").string(), "123456789012345678901");
    ASSERT_EQ(ubig128("1234567890123456789012").string(), "1234567890123456789012");
    ASSERT_EQ(ubig128("12345678901234567890123").string(), "12345678901234567890123");
    ASSERT_EQ(ubig128("123456789012345678901234").string(), "123456789012345678901234");
    ASSERT_EQ(ubig128("1234567890123456789012345").string(), "1234567890123456789012345");
    ASSERT_EQ(ubig128("12345678901234567890123456").string(), "12345678901234567890123456");
    ASSERT_EQ(ubig128("123456789012345678901234567").string(), "123456789012345678901234567");
    ASSERT_EQ(ubig128("1234567890123456789012345678").string(), "1234567890123456789012345678");
    ASSERT_EQ(ubig128("12345678901234567890123456789").string(), "12345678901234567890123456789");
    ASSERT_EQ(ubig128("123456789012345678901234567890").string(), "123456789012345678901234567890");
}

TEST(BigInt, ShiftRight)
{
    auto t = ubig128{"10000000000000000000000000000"};

    ASSERT_EQ(t, ubig128{"10000000000000000000000000000"});
    t >>= 1;
    ASSERT_EQ(t, ubig128{"5000000000000000000000000000"});
    t >>= 2;
    ASSERT_EQ(t, ubig128{"1250000000000000000000000000"});
    t >>= 3;
    ASSERT_EQ(t, ubig128{"156250000000000000000000000"});
    t >>= 4;
    ASSERT_EQ(t, ubig128{"9765625000000000000000000"});
    t >>= 5;
    ASSERT_EQ(t, ubig128{"305175781250000000000000"});
    t >>= 6;
    ASSERT_EQ(t, ubig128{"4768371582031250000000"});
    t >>= 7;
    ASSERT_EQ(t, ubig128{"37252902984619140625"});
    t >>= 8;
    ASSERT_EQ(t, ubig128{"145519152283668518"});
}

TEST(BigInt, ShiftLeft)
{
    auto t = ubig128{0b100101};
    auto u = ubig128{0b100000};

    ASSERT_EQ(t << 111, ubig128{"96057491882894311127814182090571776"});
    ASSERT_EQ(u << 111, ubig128{"83076749736557242056487941267521536"});

    t <<= 111;
    u <<= 111;
    ASSERT_EQ(t, ubig128{"96057491882894311127814182090571776"});
    ASSERT_EQ(u, ubig128{"83076749736557242056487941267521536"});

    auto v = ubig128{0x1f2e3d4c5b6a7988};
    v <<= 5;
    ASSERT_EQ(v, ubig128{"71897621192479027456"});
}

TEST(BigInt, Subtract)
{
    auto t = ubig128{"83076749736557242056487941267521536"};

    ASSERT_EQ(t - 1, ubig128{"83076749736557242056487941267521535"});

    t -= 1;
    ASSERT_EQ(t, ubig128{"83076749736557242056487941267521535"});
}

TEST(BigInt, LessThan)
{
    ASSERT_TRUE(ubig128{"1ffff"} < ubig128{"2bd40"});
}

TEST(BigInt, LessThanOrEqual)
{
    static_assert(std::numeric_limits<signed char>::min() <= big128{0});
    static_assert(std::numeric_limits<signed short>::min() <= big128{0});
    static_assert(std::numeric_limits<signed int>::min() <= big128{0});
    static_assert(std::numeric_limits<signed long>::min() <= big128{0});
    static_assert(std::numeric_limits<signed long long>::min() <= big128{0});
    static_assert(std::numeric_limits<big128>::min() <= big128{0});

    static_assert(big128{0} <= std::numeric_limits<signed char>::max());
    static_assert(big128{0} <= std::numeric_limits<signed short>::max());
    static_assert(big128{0} <= std::numeric_limits<signed int>::max());
    static_assert(big128{0} <= std::numeric_limits<signed long>::max());
    static_assert(big128{0} <= std::numeric_limits<signed long long>::max());
    static_assert(big128{0} <= std::numeric_limits<big128>::max());
}

TEST(BigInt, GreaterOrEqual)
{
    ASSERT_TRUE(ubig128{"10000000000000000000000000000"} >= 32);
    ASSERT_TRUE(ubig128{"33"} >= 32);
    ASSERT_TRUE(ubig128{"32"} >= 32);
    ASSERT_FALSE(ubig128{"31"} >= 32);
    ASSERT_FALSE(ubig128{"1"} >= 32);
    ASSERT_FALSE(ubig128{"0"} >= 32);
}

TEST(BigInt, XOR)
{
    ASSERT_EQ(ubig128{"0"} ^ ubig128{"2"}, ubig128{"2"});
    ASSERT_EQ(ubig128{"2"} ^ ubig128{"0"}, ubig128{"2"});
    ASSERT_EQ(ubig128{"2"} ^ ubig128{"2"}, ubig128{"0"});

    ASSERT_EQ(ubig128{"0"} ^ ubig128{"36893488147419103232"}, ubig128{"36893488147419103232"});
    ASSERT_EQ(ubig128{"36893488147419103232"} ^ ubig128{"0"}, ubig128{"36893488147419103232"});
    ASSERT_EQ(ubig128{"36893488147419103232"} ^ ubig128{"36893488147419103232"}, ubig128{"0"});

    ASSERT_EQ(ubig128{"0"} ^ ubig128{"36893488147419103234"}, ubig128{"36893488147419103234"});
    ASSERT_EQ(ubig128{"36893488147419103234"} ^ ubig128{"0"}, ubig128{"36893488147419103234"});
    ASSERT_EQ(ubig128{"36893488147419103234"} ^ ubig128{"36893488147419103234"}, ubig128{"0"});

    ASSERT_EQ(ubig128{"2"} ^ ubig128{"36893488147419103232"}, ubig128{"36893488147419103234"});
    ASSERT_EQ(ubig128{"36893488147419103232"} ^ ubig128{"2"}, ubig128{"36893488147419103234"});

    ASSERT_EQ(ubig128{"2"} ^ ubig128{"36893488147419103234"}, ubig128{"36893488147419103232"});
    ASSERT_EQ(ubig128{"36893488147419103234"} ^ ubig128{"2"}, ubig128{"36893488147419103232"});
}

TEST(BigInt, Divide)
{
    auto t = ubig128{"3689348814741910323200"};

    hilet[quotient, remainder] = div(t, 93);

    ASSERT_EQ(quotient, ubig128{"39670417362816240034"});
    ASSERT_EQ(remainder, 38);
}

TEST(BigInt, reciprocal)
{
    auto t = ubig128{10};
    auto r = reciprocal(static_cast<bigint<uint64_t, 4, false>>(t));
    constexpr auto r2 = reciprocal(bigint<uint64_t, 4, false>{10});

    ASSERT_EQ(r.digits[3], uint64_t{0x19999999'99999999});
    ASSERT_EQ(r.digits[2], uint64_t{0x99999999'99999999});
    ASSERT_EQ(r.digits[1], uint64_t{0x99999999'99999999});
    ASSERT_EQ(r.digits[0], uint64_t{0x99999999'99999999});
    ASSERT_EQ(r, r2);
}

TEST(BigInt, Multiply)
{
    auto t = ubig128{0x1f2e3d4c5b6a7988};

    ASSERT_EQ(t * 93, ubig128{"208952461590642173544"});
    t *= 93;
    ASSERT_EQ(t, ubig128{"208952461590642173544"});

    ASSERT_EQ(t + 25, ubig128{"208952461590642173569"});
    t += 25;
    ASSERT_EQ(t, ubig128{"208952461590642173569"});
}

TEST(BigInt, Default)
{
    {
        auto t = ubig128{1};
        auto u = ubig128{1};
        ASSERT_EQ(t, u);
    }

    {
        auto t = ubig128{"1"};
        auto u = ubig128{"1"};
        ASSERT_EQ(t, u);
    }

    {
        auto t = ubig128{"123456789012345678901234567890"};
        auto u = ubig128{"123456789012345678901234567890"};
        ASSERT_EQ(t, u);
    }

    {
        auto t = ubig128{"25"};
        auto u = ubig128{5};
        u *= 5;
        ASSERT_EQ(t, u);
    }

    {
        auto t = ubig128{"9223372036854775857"};
        auto u = ubig128{9223372036854775807};
        u += 50;
        ASSERT_EQ(t, u);
    }
}
