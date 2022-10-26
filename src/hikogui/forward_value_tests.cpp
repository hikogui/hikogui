// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "forward_value.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>

TEST(forward_value, string_literal)
{
    static_assert(std::is_same_v<hi::forward_value_t<decltype("hello world")>, char const *>);
}

TEST(forward_value, char_ptr_literal)
{
    [[maybe_unused]] char const *hello_world = "hello world";
    [[maybe_unused]] char const *const const_hello_world = "hello world";

    static_assert(std::is_same_v<hi::forward_value_t<decltype(hello_world)>, std::string>);
    static_assert(std::is_same_v<hi::forward_value_t<decltype(const_hello_world)>, std::string>);
    static_assert(std::is_same_v<hi::forward_value_t<decltype(hello_world) &>, std::string>);
    static_assert(std::is_same_v<hi::forward_value_t<decltype(const_hello_world) &>, std::string>);
}
TEST(forward_value, string_view)
{
    static_assert(std::is_same_v<hi::forward_value_t<std::string_view>, std::string>);
    static_assert(std::is_same_v<hi::forward_value_t<std::string_view const>, std::string>);
    static_assert(std::is_same_v<hi::forward_value_t<std::string_view const &>, std::string>);
    static_assert(std::is_same_v<hi::forward_value_t<std::string_view &>, std::string>);
}

TEST(forward_value, integer)
{
    static_assert(std::is_same_v<hi::forward_value_t<int>, int>);
    static_assert(std::is_same_v<hi::forward_value_t<int &>, int>);
    static_assert(std::is_same_v<hi::forward_value_t<int const &>, int>);
    static_assert(std::is_same_v<hi::forward_value_t<int *>, int *>);
    static_assert(std::is_same_v<hi::forward_value_t<int *const>, int *>);
    static_assert(std::is_same_v<hi::forward_value_t<int const *>, int const *>);
    static_assert(std::is_same_v<hi::forward_value_t<int const *const>, int const *>);
    static_assert(std::is_same_v<hi::forward_value_t<int *&>, int *>);
    static_assert(std::is_same_v<hi::forward_value_t<int *const &>, int *>);
    static_assert(std::is_same_v<hi::forward_value_t<int const *&>, int const *>);
    static_assert(std::is_same_v<hi::forward_value_t<int const *const &>, int const *>);
}

class A {
};

TEST(forward_value, class_object)
{
    static_assert(std::is_same_v<hi::forward_value_t<A>, A>);
    static_assert(std::is_same_v<hi::forward_value_t<A &>, A>);
    static_assert(std::is_same_v<hi::forward_value_t<A const &>, A>);
    static_assert(std::is_same_v<hi::forward_value_t<A *>, A *>);
    static_assert(std::is_same_v<hi::forward_value_t<A *const>, A *>);
    static_assert(std::is_same_v<hi::forward_value_t<A const *>, A const *>);
    static_assert(std::is_same_v<hi::forward_value_t<A const *const>, A const *>);
    static_assert(std::is_same_v<hi::forward_value_t<A *&>, A *>);
    static_assert(std::is_same_v<hi::forward_value_t<A *const &>, A *>);
    static_assert(std::is_same_v<hi::forward_value_t<A const *&>, A const *>);
    static_assert(std::is_same_v<hi::forward_value_t<A const *const &>, A const *>);
}
