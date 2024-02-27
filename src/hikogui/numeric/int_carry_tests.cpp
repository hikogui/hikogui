// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "int_carry.hpp"
#include <hikotest/hikotest.hpp>

template<typename T>
struct int_carry_suite : ::test::suite<int_carry_suite<T>> {
    static void register_tests()
    {
        ::test::register_test(&int_carry_suite::add_test, __FILE__, __LINE__, "add_test");
    }

    void add_test() {
        constexpr T zero = 0;
        constexpr T one = 1;
        constexpr T two = 2;
        constexpr T three = 3;
        constexpr T maximum = std::numeric_limits<T>::max();
        constexpr T high = maximum - 1;

        REQUIRE(hi::add_carry(zero, zero, zero) == std::pair(zero, zero));
        REQUIRE(hi::add_carry(zero, zero, one) == std::pair(one, zero));
        REQUIRE(hi::add_carry(zero, one, zero) == std::pair(one, zero));
        REQUIRE(hi::add_carry(zero, one, one) == std::pair(two, zero));
        REQUIRE(hi::add_carry(one, zero, zero) == std::pair(one, zero));
        REQUIRE(hi::add_carry(one, zero, one) == std::pair(two, zero));
        REQUIRE(hi::add_carry(one, one, zero) == std::pair(two, zero));
        REQUIRE(hi::add_carry(one, one, one) == std::pair(three, zero));
        REQUIRE(hi::add_carry(high, zero, zero) == std::pair(high, zero));
        REQUIRE(hi::add_carry(high, zero, one) == std::pair(maximum, zero));
        REQUIRE(hi::add_carry(high, one, zero) == std::pair(maximum, zero));
        REQUIRE(hi::add_carry(high, one, one) == std::pair(zero, one));
        REQUIRE(hi::add_carry(zero, high, zero) == std::pair(high, zero));
        REQUIRE(hi::add_carry(zero, high, one) == std::pair(maximum, zero));
        REQUIRE(hi::add_carry(one, high, zero) == std::pair(maximum, zero));
        REQUIRE(hi::add_carry(one, high, one) == std::pair(zero, one));
        REQUIRE(hi::add_carry(maximum, zero, zero) == std::pair(maximum, zero));
        REQUIRE(hi::add_carry(maximum, zero, one) == std::pair(zero, one));
        REQUIRE(hi::add_carry(maximum, one, zero) == std::pair(zero, one));
        REQUIRE(hi::add_carry(maximum, one, one) == std::pair(one, one));
        REQUIRE(hi::add_carry(zero, maximum, zero) == std::pair(maximum, zero));
        REQUIRE(hi::add_carry(zero, maximum, one) == std::pair(zero, one));
        REQUIRE(hi::add_carry(one, maximum, zero) == std::pair(zero, one));
        REQUIRE(hi::add_carry(one, maximum, one) == std::pair(one, one));

        static_assert(hi::add_carry(zero, zero, zero) == std::pair(zero, zero));
        static_assert(hi::add_carry(zero, zero, one) == std::pair(one, zero));
        static_assert(hi::add_carry(zero, one, zero) == std::pair(one, zero));
        static_assert(hi::add_carry(zero, one, one) == std::pair(two, zero));
        static_assert(hi::add_carry(one, zero, zero) == std::pair(one, zero));
        static_assert(hi::add_carry(one, zero, one) == std::pair(two, zero));
        static_assert(hi::add_carry(one, one, zero) == std::pair(two, zero));
        static_assert(hi::add_carry(one, one, one) == std::pair(three, zero));
        static_assert(hi::add_carry(high, zero, zero) == std::pair(high, zero));
        static_assert(hi::add_carry(high, zero, one) == std::pair(maximum, zero));
        static_assert(hi::add_carry(high, one, zero) == std::pair(maximum, zero));
        static_assert(hi::add_carry(high, one, one) == std::pair(zero, one));
        static_assert(hi::add_carry(zero, high, zero) == std::pair(high, zero));
        static_assert(hi::add_carry(zero, high, one) == std::pair(maximum, zero));
        static_assert(hi::add_carry(one, high, zero) == std::pair(maximum, zero));
        static_assert(hi::add_carry(one, high, one) == std::pair(zero, one));
        static_assert(hi::add_carry(maximum, zero, zero) == std::pair(maximum, zero));
        static_assert(hi::add_carry(maximum, zero, one) == std::pair(zero, one));
        static_assert(hi::add_carry(maximum, one, zero) == std::pair(zero, one));
        static_assert(hi::add_carry(maximum, one, one) == std::pair(one, one));
        static_assert(hi::add_carry(zero, maximum, zero) == std::pair(maximum, zero));
        static_assert(hi::add_carry(zero, maximum, one) == std::pair(zero, one));
        static_assert(hi::add_carry(one, maximum, zero) == std::pair(zero, one));
        static_assert(hi::add_carry(one, maximum, one) == std::pair(one, one));
    }

};

inline auto _hikotest_registered_int_carry_suite_uchar = &::test::register_suite<int_carry_suite<unsigned char>>();
inline auto _hikotest_registered_int_carry_suite_ushort = &::test::register_suite<int_carry_suite<unsigned short>>();
inline auto _hikotest_registered_int_carry_suite_uint = &::test::register_suite<int_carry_suite<unsigned int>>();
inline auto _hikotest_registered_int_carry_suite_ulong = &::test::register_suite<int_carry_suite<unsigned long>>();
inline auto _hikotest_registered_int_carry_suite_ulonglong = &::test::register_suite<int_carry_suite<unsigned long long>>();

