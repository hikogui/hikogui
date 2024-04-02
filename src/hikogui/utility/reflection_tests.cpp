// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "reflection.hpp"
#include <hikotest/hikotest.hpp>

namespace reflection_suite_ns {

struct empty_type {};

struct int_type {
    int a;
};

struct char_int_type {
    char a;
    int b;
};

struct int_char_type {
    int a;
    char b;
};

struct non_trivial_type {
    int a;
    char b;

    constexpr non_trivial_type() : a(1), b('z') {}
};

}

template<>
struct hi::number_of_data_members<reflection_suite_ns::non_trivial_type> : std::integral_constant<size_t, 2> {};

TEST_SUITE(reflection_suite) {

TEST_CASE(count_members)
{
    static_assert(hi::number_of_data_members_v<reflection_suite_ns::empty_type> == 0);
    static_assert(hi::number_of_data_members_v<reflection_suite_ns::int_type> == 1);
    static_assert(hi::number_of_data_members_v<reflection_suite_ns::char_int_type> == 2);
    static_assert(hi::number_of_data_members_v<reflection_suite_ns::int_char_type> == 2);
    static_assert(hi::number_of_data_members_v<reflection_suite_ns::non_trivial_type> == 2);
}

TEST_CASE(member_type_rvalueref)
{
    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(reflection_suite_ns::int_type{})), int&&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(reflection_suite_ns::char_int_type{})), char&&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<1>(reflection_suite_ns::char_int_type{})), int&&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(reflection_suite_ns::int_char_type{})), int&&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<1>(reflection_suite_ns::int_char_type{})), char&&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(reflection_suite_ns::non_trivial_type{})), int&&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<1>(reflection_suite_ns::non_trivial_type{})), char&&>);
}

TEST_CASE(member_type_lvalueref)
{
    auto int_value = reflection_suite_ns::int_type{};
    auto char_int_value = reflection_suite_ns::char_int_type{};
    auto int_char_value = reflection_suite_ns::int_char_type{};
    auto non_trivial_value = reflection_suite_ns::non_trivial_type{};

    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(int_value)), int&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(char_int_value)), char&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<1>(char_int_value)), int&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(int_char_value)), int&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<1>(int_char_value)), char&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(non_trivial_value)), int&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<1>(non_trivial_value)), char&>);
}

TEST_CASE(member_type_lvalueconstref)
{
    auto const int_value = reflection_suite_ns::int_type{};
    auto const char_int_value = reflection_suite_ns::char_int_type{};
    auto const int_char_value = reflection_suite_ns::int_char_type{};
    auto const non_trivial_value = reflection_suite_ns::non_trivial_type{};

    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(int_value)), int const&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(char_int_value)), char const&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<1>(char_int_value)), int const&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(int_char_value)), int const&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<1>(int_char_value)), char const&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<0>(non_trivial_value)), int const&>);
    static_assert(std::is_same_v<decltype(hi::get_data_member<1>(non_trivial_value)), char const&>);
}

TEST_CASE(member_value_rvalueref)
{
    static_assert(hi::get_data_member<0>(reflection_suite_ns::int_type{42}) == 42);
    static_assert(hi::get_data_member<0>(reflection_suite_ns::char_int_type{'a', 43}) == 'a');
    static_assert(hi::get_data_member<1>(reflection_suite_ns::char_int_type{'b', 44}) == 44);
    static_assert(hi::get_data_member<0>(reflection_suite_ns::int_char_type{45, 'c'}) == 45);
    static_assert(hi::get_data_member<1>(reflection_suite_ns::int_char_type{46, 'd'}) == 'd');
    static_assert(hi::get_data_member<0>(reflection_suite_ns::non_trivial_type{}) == 1);
    static_assert(hi::get_data_member<1>(reflection_suite_ns::non_trivial_type{}) == 'z');
}

TEST_CASE(member_value_lvalueref)
{
    auto int_value = reflection_suite_ns::int_type{42};
    auto char_int_value = reflection_suite_ns::char_int_type{'a', 43};
    auto int_char_value = reflection_suite_ns::int_char_type{44, 'b'};
    auto non_trivial_value = reflection_suite_ns::non_trivial_type{};

    REQUIRE(hi::get_data_member<0>(int_value) == 42);
    hi::get_data_member<0>(int_value) = 5;
    REQUIRE(hi::get_data_member<0>(int_value) == 5);

    REQUIRE(hi::get_data_member<0>(char_int_value) == 'a');
    REQUIRE(hi::get_data_member<1>(char_int_value) == 43);
    hi::get_data_member<0>(char_int_value) = 'y';
    hi::get_data_member<1>(char_int_value) = 9;
    REQUIRE(hi::get_data_member<0>(char_int_value) == 'y');
    REQUIRE(hi::get_data_member<1>(char_int_value) == 9);

    REQUIRE(hi::get_data_member<0>(int_char_value) == 44);
    REQUIRE(hi::get_data_member<1>(int_char_value) == 'b');
    hi::get_data_member<0>(int_char_value) = 10;
    hi::get_data_member<1>(int_char_value) = 'x';
    REQUIRE(hi::get_data_member<0>(int_char_value) == 10);
    REQUIRE(hi::get_data_member<1>(int_char_value) == 'x');

    REQUIRE(hi::get_data_member<0>(non_trivial_value) == 1);
    REQUIRE(hi::get_data_member<1>(non_trivial_value) == 'z');
    hi::get_data_member<0>(non_trivial_value) = 11;
    hi::get_data_member<1>(non_trivial_value) = 'g';
    REQUIRE(hi::get_data_member<0>(non_trivial_value) == 11);
    REQUIRE(hi::get_data_member<1>(non_trivial_value) == 'g');
}

TEST_CASE(member_value_lvalueconstref)
{
    auto int_value = reflection_suite_ns::int_type{42};
    auto char_int_value = reflection_suite_ns::char_int_type{'a', 43};
    auto int_char_value = reflection_suite_ns::int_char_type{44, 'b'};
    auto non_trivial_value = reflection_suite_ns::non_trivial_type{};

    REQUIRE(hi::get_data_member<0>(int_value) == 42);
    REQUIRE(hi::get_data_member<0>(char_int_value) == 'a');
    REQUIRE(hi::get_data_member<1>(char_int_value) == 43);
    REQUIRE(hi::get_data_member<0>(int_char_value) == 44);
    REQUIRE(hi::get_data_member<1>(int_char_value) == 'b');
    REQUIRE(hi::get_data_member<0>(non_trivial_value) == 1);
    REQUIRE(hi::get_data_member<1>(non_trivial_value) == 'z');
}

#if HI_COMPILER == HI_CC_MSVC
TEST_CASE(type_name)
{
    REQUIRE(hi::type_name<int>() == hi::fixed_string{"int"});
    REQUIRE(hi::type_name<reflection_suite_ns::int_type>() == hi::fixed_string{"reflection_suite_ns::int_type"});
    REQUIRE(hi::type_name<std::string>() == hi::fixed_string{"std::string"});
    REQUIRE(hi::type_name<std::string &>() == hi::fixed_string{"std::string&"});
    REQUIRE(hi::type_name<std::string const&>() == hi::fixed_string{"const std::string&"});
    REQUIRE(hi::type_name<std::vector<int>>() == hi::fixed_string{"std::vector<int,std::allocator<int>>"});
    REQUIRE(hi::type_name<std::vector<int>&>() == hi::fixed_string{"std::vector<int,std::allocator<int>>&"});
    // XXX need proper tokenizer for this to be correct.
    //REQUIRE(hi::type_name<std::vector<int *>&>() == hi::fixed_string{"std::vector<int*,std::allocator<int*>>&"});
}
#endif

};
