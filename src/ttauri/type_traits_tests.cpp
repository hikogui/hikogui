// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "type_traits.hpp"
#include "concepts.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>

class A {
};

class B : public A {
};

class C : public A {
};

TEST(type_traits, decayed_base_of)
{
    static_assert(std::is_base_of_v<A, A>);
    static_assert(std::is_base_of_v<A, B>);
    static_assert(std::is_base_of_v<A, C>);
    static_assert(!std::is_base_of_v<B, A>);
    static_assert(!std::is_base_of_v<C, A>);

    static_assert(tt::is_decayed_base_of_v<A, A>);
    static_assert(tt::is_decayed_base_of_v<A, B>);
    static_assert(tt::is_decayed_base_of_v<A, C>);
    static_assert(!tt::is_decayed_base_of_v<B, A>);
    static_assert(!tt::is_decayed_base_of_v<C, A>);

    static_assert(tt::is_decayed_base_of_v<A &, A>);
    static_assert(tt::is_decayed_base_of_v<A &, B>);
    static_assert(tt::is_decayed_base_of_v<A &, C>);
    static_assert(!tt::is_decayed_base_of_v<B &, A>);
    static_assert(!tt::is_decayed_base_of_v<C &, A>);

    static_assert(tt::is_decayed_base_of_v<A, A &>);
    static_assert(tt::is_decayed_base_of_v<A, B &>);
    static_assert(tt::is_decayed_base_of_v<A, C &>);
    static_assert(!tt::is_decayed_base_of_v<B, A &>);
    static_assert(!tt::is_decayed_base_of_v<C, A &>);

    static_assert(tt::is_decayed_base_of_v<A &, A &>);
    static_assert(tt::is_decayed_base_of_v<A &, B &>);
    static_assert(tt::is_decayed_base_of_v<A &, C &>);
    static_assert(!tt::is_decayed_base_of_v<B &, A &>);
    static_assert(!tt::is_decayed_base_of_v<C &, A &>);
}

template<tt::forward_of<std::string> Text>
std::string forward_of_test_func(Text &&text)
{
    return std::forward<Text>(text);
}

std::string forward_of_test()
{
    auto t = std::string{};

    t = forward_of_test_func(std::string{"hello world"});

    auto text = std::string{"hello world"};
    t = forward_of_test_func(text);

    auto const const_text = std::string{"hello world"};
    t = forward_of_test_func(const_text);
    return t;
}
