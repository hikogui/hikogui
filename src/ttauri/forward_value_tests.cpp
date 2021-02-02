// Copyright 2020 Pokitec
// All rights reserved.

#include "forward_value.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>


TEST(forward_value, string_literal)
{
    static_assert(std::is_same_v<tt::forward_value_t<decltype("hello world")>, char const *>);
}

TEST(forward_value, string_view)
{
    static_assert(std::is_same_v<tt::forward_value_t<std::string_view>, std::string>);
}

TEST(forward_value, integer)
{
    static_assert(std::is_same_v<tt::forward_value_t<int>, int>);
    static_assert(std::is_same_v<tt::forward_value_t<int &>, int>);
    static_assert(std::is_same_v<tt::forward_value_t<int const &>, int>);
    static_assert(std::is_same_v<tt::forward_value_t<int *>, int *>);
    static_assert(std::is_same_v<tt::forward_value_t<int const *>, int const *>);
}

class A{};

TEST(forward_value, class_object)
{
    static_assert(std::is_same_v<tt::forward_value_t<A>, A>);
    static_assert(std::is_same_v<tt::forward_value_t<A &>, A>);
    static_assert(std::is_same_v<tt::forward_value_t<A const &>, A>);
    static_assert(std::is_same_v<tt::forward_value_t<A *>, A *>);
    static_assert(std::is_same_v<tt::forward_value_t<A const *>, A const *>);
}
