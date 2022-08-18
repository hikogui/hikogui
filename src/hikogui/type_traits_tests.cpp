// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "type_traits.hpp"
#include "concepts.hpp"
#include "fixed_string.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>

class A {};

class B : public A {};

class C : public A {};

TEST(type_traits, decayed_base_of)
{
    static_assert(std::is_base_of_v<A, A>);
    static_assert(std::is_base_of_v<A, B>);
    static_assert(std::is_base_of_v<A, C>);
    static_assert(!std::is_base_of_v<B, A>);
    static_assert(!std::is_base_of_v<C, A>);

    static_assert(hi::is_decayed_base_of_v<A, A>);
    static_assert(hi::is_decayed_base_of_v<A, B>);
    static_assert(hi::is_decayed_base_of_v<A, C>);
    static_assert(!hi::is_decayed_base_of_v<B, A>);
    static_assert(!hi::is_decayed_base_of_v<C, A>);

    static_assert(hi::is_decayed_base_of_v<A&, A>);
    static_assert(hi::is_decayed_base_of_v<A&, B>);
    static_assert(hi::is_decayed_base_of_v<A&, C>);
    static_assert(!hi::is_decayed_base_of_v<B&, A>);
    static_assert(!hi::is_decayed_base_of_v<C&, A>);

    static_assert(hi::is_decayed_base_of_v<A, A&>);
    static_assert(hi::is_decayed_base_of_v<A, B&>);
    static_assert(hi::is_decayed_base_of_v<A, C&>);
    static_assert(!hi::is_decayed_base_of_v<B, A&>);
    static_assert(!hi::is_decayed_base_of_v<C, A&>);

    static_assert(hi::is_decayed_base_of_v<A&, A&>);
    static_assert(hi::is_decayed_base_of_v<A&, B&>);
    static_assert(hi::is_decayed_base_of_v<A&, C&>);
    static_assert(!hi::is_decayed_base_of_v<B&, A&>);
    static_assert(!hi::is_decayed_base_of_v<C&, A&>);
}

namespace my {
struct simple {
    int foo;
    std::string bar;
};
} // namespace my

// clang-format off
template<>
struct hi::selector<my::simple> {
    template<hi::basic_fixed_string> auto& get(my::simple&) const noexcept;
    template<hi::basic_fixed_string> auto const& get(my::simple const&) const noexcept;

    template<> auto& get<"foo">(my::simple& rhs) const noexcept { return rhs.foo; }
    template<> auto const& get<"foo">(my::simple const& rhs) const noexcept { return rhs.foo; }

    template<> auto& get<"bar">(my::simple& rhs) const noexcept { return rhs.bar; }
    template<> auto const& get<"bar">(my::simple const& rhs) const noexcept { return rhs.bar; }
};
// clang-format on

TEST(type_traits, selector)
{
    auto tmp = my::simple{42, "hello world"};

    ASSERT_EQ(hi::selector<my::simple>{}.get<"foo">(tmp), 42);
    ASSERT_EQ(hi::selector<my::simple>{}.get<"bar">(tmp), "hello world");
}
