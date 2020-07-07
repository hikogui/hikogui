// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/foundation/TT5.hpp"
#include "ttauri/foundation/exceptions.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>
#include <cstdint>

using namespace std;
using namespace tt;

template<typename T>
void test_single_RoundTrip(std::string const &str)
{
    auto value = T{};

    ASSERT_NO_THROW(value = tt5_encode<T>(str));
    ASSERT_EQ(tt5_decode(value), str);
}

TEST(TT5, RoundTrip64) {
    test_single_RoundTrip<uint64_t>("hello-world");
    test_single_RoundTrip<uint64_t>("Hello-world");
    test_single_RoundTrip<uint64_t>("hEllo-world");

    test_single_RoundTrip<uint64_t>("foobar");
    test_single_RoundTrip<uint64_t>("fooBar");
    test_single_RoundTrip<uint64_t>("foOBar");
    test_single_RoundTrip<uint64_t>("foOBAr");
    test_single_RoundTrip<uint64_t>("fOOBAr");
    test_single_RoundTrip<uint64_t>("fOOBAR");
    test_single_RoundTrip<uint64_t>("FOOBAR");

    test_single_RoundTrip<uint64_t>("foo-bar");
    test_single_RoundTrip<uint64_t>("foo-Bar");
    test_single_RoundTrip<uint64_t>("foO-Bar");
    test_single_RoundTrip<uint64_t>("foO-BAr");
    test_single_RoundTrip<uint64_t>("fOO-BAr");
    test_single_RoundTrip<uint64_t>("fOO-BAR");
    test_single_RoundTrip<uint64_t>("FOO-BAR");

    test_single_RoundTrip<uint64_t>("foo1bar");
    test_single_RoundTrip<uint64_t>("foo2Bar");
    test_single_RoundTrip<uint64_t>("foO3Bar");
    test_single_RoundTrip<uint64_t>("foO4BAr");
    test_single_RoundTrip<uint64_t>("fOO5BAr");

    test_single_RoundTrip<uint64_t>("foo\x07zar");
    test_single_RoundTrip<uint64_t>("foo\xfeZar");

}

TEST(TT5, RoundTrip128) {
    ASSERT_THROW((void)tt5_encode<ubig128>("abcdefghijklmnopqrstuvwxyz"), parse_error);
    test_single_RoundTrip<ubig128>("abcdefghijklmnopqrstuvwxy");
    test_single_RoundTrip<ubig128>("abcz_.-");
    ASSERT_THROW((void)tt5_encode<ubig128>("ABCDEFGHIJKLMNOPQRSTUVWX"), parse_error);
    test_single_RoundTrip<ubig128>("ABCDEFGHIJKLMNOPQRSTUVW");
    test_single_RoundTrip<ubig128>("ABCXYZ_.-");
    ASSERT_THROW((void)tt5_encode<ubig128>("0123456789,:;/\n_.-123456"), parse_error);
    test_single_RoundTrip<ubig128>("0123456789,:;/\n_.-12345");

    test_single_RoundTrip<ubig128>("hello-whole-world");
    test_single_RoundTrip<ubig128>("Hello-whole-world");
    test_single_RoundTrip<ubig128>("hEllo-whole-world");

    test_single_RoundTrip<ubig128>("fooBar");
    test_single_RoundTrip<ubig128>("foOBar");
    test_single_RoundTrip<ubig128>("foOBAr");
    test_single_RoundTrip<ubig128>("fOOBAr");
    test_single_RoundTrip<ubig128>("fOOBAR");
    test_single_RoundTrip<ubig128>("FOOBAR");
    test_single_RoundTrip<ubig128>("foobar");

    test_single_RoundTrip<ubig128>("foo-bar");
    test_single_RoundTrip<ubig128>("foo-Bar");
    test_single_RoundTrip<ubig128>("foO-Bar");
    test_single_RoundTrip<ubig128>("foO-BAr");
    test_single_RoundTrip<ubig128>("fOO-BAr");
    test_single_RoundTrip<ubig128>("fOO-BAR");
    test_single_RoundTrip<ubig128>("FOO-BAR");

    test_single_RoundTrip<ubig128>("foo1bar");
    test_single_RoundTrip<ubig128>("foo2Bar");
    test_single_RoundTrip<ubig128>("foO3Bar");
    test_single_RoundTrip<ubig128>("foO4BAr");
    test_single_RoundTrip<ubig128>("fOO5BAr");
    test_single_RoundTrip<ubig128>("fOO5BAR");
    test_single_RoundTrip<ubig128>("FOO5BAR");

    test_single_RoundTrip<ubig128>("foo\x07zar");
    test_single_RoundTrip<ubig128>("foo\xfeZar");
}
