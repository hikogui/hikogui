// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/bigint.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace std;
using namespace TTauri;

TEST(BigInt, Construct) {
    ASSERT_EQ(uint128_t("1").string(), "1");
    ASSERT_EQ(uint128_t("10").string(), "10");
    ASSERT_EQ(uint128_t("100").string(), "100");
    ASSERT_EQ(uint128_t("1000").string(), "1000");
    ASSERT_EQ(uint128_t("10000").string(), "10000");
    ASSERT_EQ(uint128_t("100000").string(), "100000");
    ASSERT_EQ(uint128_t("1000000").string(), "1000000");
    ASSERT_EQ(uint128_t("10000000").string(), "10000000");
    ASSERT_EQ(uint128_t("100000000").string(), "100000000");
    ASSERT_EQ(uint128_t("1000000000").string(), "1000000000");
    ASSERT_EQ(uint128_t("10000000000").string(), "10000000000");
    ASSERT_EQ(uint128_t("100000000000").string(), "100000000000");
    ASSERT_EQ(uint128_t("1000000000000").string(), "1000000000000");
    ASSERT_EQ(uint128_t("10000000000000").string(), "10000000000000");
    ASSERT_EQ(uint128_t("100000000000000").string(), "100000000000000");
    ASSERT_EQ(uint128_t("1000000000000000").string(), "1000000000000000");
    ASSERT_EQ(uint128_t("10000000000000000").string(), "10000000000000000");
    ASSERT_EQ(uint128_t("100000000000000000").string(), "100000000000000000");
    ASSERT_EQ(uint128_t("1000000000000000000").string(), "1000000000000000000");
    ASSERT_EQ(uint128_t("10000000000000000000").string(), "10000000000000000000");
    ASSERT_EQ(uint128_t("100000000000000000000").string(), "100000000000000000000");
    ASSERT_EQ(uint128_t("1000000000000000000000").string(), "1000000000000000000000");
    ASSERT_EQ(uint128_t("10000000000000000000000").string(), "10000000000000000000000");
    ASSERT_EQ(uint128_t("100000000000000000000000").string(), "100000000000000000000000");
    ASSERT_EQ(uint128_t("1000000000000000000000000").string(), "1000000000000000000000000");
    ASSERT_EQ(uint128_t("10000000000000000000000000").string(), "10000000000000000000000000");

    ASSERT_EQ(uint128_t("12").string(), "12");
    ASSERT_EQ(uint128_t("123").string(), "123");
    ASSERT_EQ(uint128_t("1234").string(), "1234");
    ASSERT_EQ(uint128_t("12345").string(), "12345");
    ASSERT_EQ(uint128_t("123456").string(), "123456");
    ASSERT_EQ(uint128_t("1234567").string(), "1234567");
    ASSERT_EQ(uint128_t("12345678").string(), "12345678");
    ASSERT_EQ(uint128_t("123456789").string(), "123456789");
    ASSERT_EQ(uint128_t("1234567890").string(), "1234567890");
    ASSERT_EQ(uint128_t("12345678901").string(), "12345678901");
    ASSERT_EQ(uint128_t("123456789012").string(), "123456789012");
    ASSERT_EQ(uint128_t("1234567890123").string(), "1234567890123");
    ASSERT_EQ(uint128_t("12345678901234").string(), "12345678901234");
    ASSERT_EQ(uint128_t("123456789012345").string(), "123456789012345");
    ASSERT_EQ(uint128_t("1234567890123456").string(), "1234567890123456");
    ASSERT_EQ(uint128_t("12345678901234567").string(), "12345678901234567");
    ASSERT_EQ(uint128_t("123456789012345678").string(), "123456789012345678");
    ASSERT_EQ(uint128_t("1234567890123456789").string(), "1234567890123456789");
    ASSERT_EQ(uint128_t("12345678901234567890").string(), "12345678901234567890");
    ASSERT_EQ(uint128_t("123456789012345678901").string(), "123456789012345678901");
    ASSERT_EQ(uint128_t("1234567890123456789012").string(), "1234567890123456789012");
    ASSERT_EQ(uint128_t("12345678901234567890123").string(), "12345678901234567890123");
    ASSERT_EQ(uint128_t("123456789012345678901234").string(), "123456789012345678901234");
    ASSERT_EQ(uint128_t("1234567890123456789012345").string(), "1234567890123456789012345");
    ASSERT_EQ(uint128_t("12345678901234567890123456").string(), "12345678901234567890123456");
    ASSERT_EQ(uint128_t("123456789012345678901234567").string(), "123456789012345678901234567");
    ASSERT_EQ(uint128_t("1234567890123456789012345678").string(), "1234567890123456789012345678");
    ASSERT_EQ(uint128_t("12345678901234567890123456789").string(), "12345678901234567890123456789");
    ASSERT_EQ(uint128_t("123456789012345678901234567890").string(), "123456789012345678901234567890");
}

TEST(BigInt, ShiftRight) {
    auto t = uint128_t{"10000000000000000000000000000"};

    ASSERT_EQ(t, uint128_t{"10000000000000000000000000000"});
    t >>= 1;
    ASSERT_EQ(t, uint128_t{"5000000000000000000000000000"});
    t >>= 2;
    ASSERT_EQ(t, uint128_t{"1250000000000000000000000000"});
    t >>= 3;
    ASSERT_EQ(t, uint128_t{"156250000000000000000000000"});
    t >>= 4;
    ASSERT_EQ(t, uint128_t{"9765625000000000000000000"});
    t >>= 5;
    ASSERT_EQ(t, uint128_t{"305175781250000000000000"});
    t >>= 6;
    ASSERT_EQ(t, uint128_t{"4768371582031250000000"});
    t >>= 7;
    ASSERT_EQ(t, uint128_t{"37252902984619140625"});
    t >>= 8;
    ASSERT_EQ(t, uint128_t{"145519152283668518"});
}

TEST(BigInt, ShiftLeft) {
    auto t = uint128_t{0b100101};
    auto u = uint128_t{0b100000};

    ASSERT_EQ(t << 111, uint128_t{"96057491882894311127814182090571776"});
    ASSERT_EQ(u << 111, uint128_t{"83076749736557242056487941267521536"});

    t <<= 111;
    u <<= 111;
    ASSERT_EQ(t, uint128_t{"96057491882894311127814182090571776"});
    ASSERT_EQ(u, uint128_t{"83076749736557242056487941267521536"});

    auto v = uint128_t{0x1f2e3d4c5b6a7988};
    v <<= 5;
    ASSERT_EQ(v, uint128_t{"71897621192479027456"});
}

TEST(BigInt, Subtract) {
    auto t = uint128_t{"83076749736557242056487941267521536"};

    ASSERT_EQ(t - 1, uint128_t{"83076749736557242056487941267521535"});

    t -= 1;
    ASSERT_EQ(t, uint128_t{"83076749736557242056487941267521535"});
}

TEST(BigInt, LessThan) {
    ASSERT_TRUE(uint128_t{"1ffff"} < uint128_t{"2bd40"});

}

TEST(BigInt, GreaterOrEqual) {
    ASSERT_TRUE(uint128_t{"10000000000000000000000000000"} >= 32);
    ASSERT_TRUE(uint128_t{"33"} >= 32);
    ASSERT_TRUE(uint128_t{"32"} >= 32);
    ASSERT_FALSE(uint128_t{"31"} >= 32);
    ASSERT_FALSE(uint128_t{"1"} >= 32);
    ASSERT_FALSE(uint128_t{"0"} >= 32);
}

TEST(BigInt, XOR) {
    ASSERT_EQ(uint128_t{"0"} ^ uint128_t{"2"}, uint128_t{"2"});
    ASSERT_EQ(uint128_t{"2"} ^ uint128_t{"0"}, uint128_t{"2"});
    ASSERT_EQ(uint128_t{"2"} ^ uint128_t{"2"}, uint128_t{"0"});

    ASSERT_EQ(uint128_t{"0"} ^ uint128_t{"36893488147419103232"}, uint128_t{"36893488147419103232"});
    ASSERT_EQ(uint128_t{"36893488147419103232"} ^ uint128_t{"0"}, uint128_t{"36893488147419103232"});
    ASSERT_EQ(uint128_t{"36893488147419103232"} ^ uint128_t{"36893488147419103232"}, uint128_t{"0"});

    ASSERT_EQ(uint128_t{"0"} ^ uint128_t{"36893488147419103234"}, uint128_t{"36893488147419103234"});
    ASSERT_EQ(uint128_t{"36893488147419103234"} ^ uint128_t{"0"}, uint128_t{"36893488147419103234"});
    ASSERT_EQ(uint128_t{"36893488147419103234"} ^ uint128_t{"36893488147419103234"}, uint128_t{"0"});

    ASSERT_EQ(uint128_t{"2"} ^ uint128_t{"36893488147419103232"}, uint128_t{"36893488147419103234"});
    ASSERT_EQ(uint128_t{"36893488147419103232"} ^ uint128_t{"2"}, uint128_t{"36893488147419103234"});

    ASSERT_EQ(uint128_t{"2"} ^ uint128_t{"36893488147419103234"}, uint128_t{"36893488147419103232"});
    ASSERT_EQ(uint128_t{"36893488147419103234"} ^ uint128_t{"2"}, uint128_t{"36893488147419103232"});
}

TEST(BigInt, Divide) {
    auto t = uint128_t{"3689348814741910323200"};

    let [quotient, remainder] = div(t, 93);

    ASSERT_EQ(quotient, uint128_t{"39670417362816240034"});
    ASSERT_EQ(remainder, 38);
}

TEST(BigInt, Multiply) {
    auto t = uint128_t{0x1f2e3d4c5b6a7988};

    ASSERT_EQ(t * 93, uint128_t{"208952461590642173544"});
    t *= 93;
    ASSERT_EQ(t, uint128_t{"208952461590642173544"});

    ASSERT_EQ(t + 25, uint128_t{"208952461590642173569"});
    t += 25;
    ASSERT_EQ(t, uint128_t{"208952461590642173569"});
}

TEST(BigInt, Default) {
    {
        auto t = uint128_t{1};
        auto u = uint128_t{1};
        ASSERT_EQ(t, u);
    }

    {
        auto t = uint128_t{"1"};
        auto u = uint128_t{"1"};
        ASSERT_EQ(t, u);
    }

    {
        auto t = uint128_t{"123456789012345678901234567890"};
        auto u = uint128_t{"123456789012345678901234567890"};
        ASSERT_EQ(t, u);
    }

    {
        auto t = uint128_t{"25"};
        auto u = uint128_t{5};
        u *= 5;
        ASSERT_EQ(t, u);
    }

    {
        auto t = uint128_t{"9223372036854775857"};
        auto u = uint128_t{9223372036854775807};
        u+= 50;
        ASSERT_EQ(t, u);
    }

}