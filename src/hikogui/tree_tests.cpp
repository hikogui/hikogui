// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "tree.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

static auto make_test()
{
    auto r = hi::tree<int, std::string>{};

    r[std::array<int,0>{}] = "root";
    r[std::array{1}] = "hello";
    r[std::array{1, 1}] = "city";
    r[std::array{1, 2}] = "state";
    r[std::array{1, 3}] = "country";
    r[std::array{1, 4}] = "world";
    r[std::array{2}] = "foo";
    r[std::array{2, 1}] = "bar";
    r[std::array{2, 2}] = "baz";

    return r;
}

TEST(tree, read)
{
    auto t = make_test();
    hilet _ = std::array<int,0>{};
    hilet _1 = std::array{1};
    hilet _1_2 = std::array{1, 2};
    hilet _2_2 = std::array{2, 2};
    hilet _3_2 = std::array{3, 2};

    ASSERT_EQ(t[_], "root");
    ASSERT_EQ(t[_1], "hello");
    ASSERT_EQ(t[_1_2], "state");
    ASSERT_EQ(t[_2_2], "baz");
    ASSERT_EQ(t[_3_2], "");

    auto const& ct = t;
    ASSERT_EQ(ct[_], "root");
    ASSERT_EQ(ct[_1], "hello");
    ASSERT_EQ(ct[_1_2], "state");
    ASSERT_EQ(ct[_2_2], "baz");
    // std::array{3, 2} was created earlier.
    ASSERT_EQ(ct[_3_2], "");
}

TEST(tree, walk)
{
    auto t = make_test();

    {
        auto result = std::string{};
        t.walk([&result](std::string const& e) {
            result += '.';
            result += e;
        });
        ASSERT_EQ(result, ".root.hello.city.state.country.world.foo.bar.baz");
    }

    {
        auto result = std::string{};
        t.walk(std::array<int,0>{}, [&result](std::string const& e) {
            result += '.';
            result += e;
        });
        ASSERT_EQ(result, ".root.hello.city.state.country.world.foo.bar.baz");
    }

    {
        auto result = std::string{};
        t.walk(std::array{1}, [&result](std::string const& e) {
            result += '.';
            result += e;
        });
        ASSERT_EQ(result, ".hello.city.state.country.world");
    }

    {
        auto result = std::string{};
        t.walk(std::array{1, 2}, [&result](std::string const& e) {
            result += '.';
            result += e;
        });
        ASSERT_EQ(result, ".state");
    }
}

TEST(tree, walk_including_path)
{
    auto t = make_test();

    {
        auto result = std::string{};
        t.walk_including_path(std::array<int, 0>{}, [&result](std::string const& e) {
            result += '.';
            result += e;
        });
        ASSERT_EQ(result, ".root.hello.city.state.country.world.foo.bar.baz");
    }

    {
        auto result = std::string{};
        t.walk_including_path(std::array{1}, [&result](std::string const& e) {
            result += '.';
            result += e;
        });
        ASSERT_EQ(result, ".root.hello.city.state.country.world");
    }

    {
        auto result = std::string{};
        t.walk_including_path(std::array{1, 2}, [&result](std::string const& e) {
            result += '.';
            result += e;
        });
        ASSERT_EQ(result, ".root.hello.state");
    }
}
