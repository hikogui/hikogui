// Copyright 2020 Pokitec
// All rights reserved.

#include "type_traits.hpp"
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