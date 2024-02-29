// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "fixed_string.hpp"
#include "type_traits.hpp"
#include "concepts.hpp"
#include <hikotest/hikotest.hpp>

namespace type_traits_suite_ns {

struct simple {
    int foo;
    std::string bar;
};

} // namespace type_traits_suite_ns

// clang-format off
template<>
struct hi::selector<type_traits_suite_ns::simple> {
    template<hi::fixed_string> auto& get(type_traits_suite_ns::simple&) const noexcept;
    template<hi::fixed_string> auto const& get(type_traits_suite_ns::simple const&) const noexcept;

    template<> auto& get<"foo">(type_traits_suite_ns::simple& rhs) const noexcept { return rhs.foo; }
    template<> auto const& get<"foo">(type_traits_suite_ns::simple const& rhs) const noexcept { return rhs.foo; }

    template<> auto& get<"bar">(type_traits_suite_ns::simple& rhs) const noexcept { return rhs.bar; }
    template<> auto const& get<"bar">(type_traits_suite_ns::simple const& rhs) const noexcept { return rhs.bar; }
};
// clang-format on

TEST_SUITE(type_traits_suite) {

class A {};

class B : public A {};

class C : public A {};

TEST_CASE(decayed_base_of)
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

TEST_CASE(forward_of)
{
    static_assert(hi::is_forward_of_v<std::string, std::string>);
    static_assert(hi::is_forward_of_v<std::string const&, std::string>);

    static_assert(hi::is_forward_of_v<std::hash<int>, size_t(int)>);
    static_assert(hi::is_forward_of_v<std::function<size_t(int)>, size_t(int)>);
    static_assert(hi::is_forward_of_v<std::function<size_t(int)> const&, size_t(int)>);
    static_assert(hi::is_forward_of_v<
                  decltype([](int) -> size_t {
                      return 1;
                  }) const&,
                  size_t(int)>);
}

TEST_CASE(selector)
{
    auto tmp = type_traits_suite_ns::simple{42, "hello world"};

    REQUIRE(hi::selector<type_traits_suite_ns::simple>{}.get<"foo">(tmp) == 42);
    REQUIRE(hi::selector<type_traits_suite_ns::simple>{}.get<"bar">(tmp) == "hello world");
}

}; // TEST_SUITE(type_traits_suite)
