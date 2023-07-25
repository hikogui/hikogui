// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "fixed_string.hpp"
#include "type_traits.hpp"
#include "../macros.hpp"
#include "concepts.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>
#include <utility>

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

TEST(type_traits, forward_of)
{
    static_assert(hi::is_forward_of_v<std::string, std::string>);
    static_assert(hi::is_forward_of_v<std::string const &, std::string>);

    static_assert(hi::is_forward_of_v<std::hash<int>, size_t(int)>);
    static_assert(hi::is_forward_of_v<std::function<size_t(int)>, size_t(int)>);
    static_assert(hi::is_forward_of_v<std::function<size_t(int)> const&, size_t(int)>);
    static_assert(hi::is_forward_of_v<decltype([](int) -> size_t { return 1;}) const&, size_t(int)>);
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
    template<hi::fixed_string> auto& get(my::simple&) const noexcept;
    template<hi::fixed_string> auto const& get(my::simple const&) const noexcept;

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
