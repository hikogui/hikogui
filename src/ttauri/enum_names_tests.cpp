// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "enum_names.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

enum class continues_type {
    off,
    on,
    maybe
};

constexpr auto continues_type_names = tt::enum_names{
    continues_type::on, "on",
    continues_type::off, "off",
    continues_type::maybe, "maybe",
};

enum class offset_type {
    maybe = -1,
    off,
    on,
};

constexpr auto offset_type_names = tt::enum_names{
    offset_type::on, "on",
    offset_type::off, "off",
    offset_type::maybe, "maybe",
};

enum class discontinues_type {
    maybe = -2,
    off = 0,
    on = 1,
};

constexpr auto discontinues_type_names = tt::enum_names{
    discontinues_type::on, "on",
    discontinues_type::off, "off",
    discontinues_type::maybe, "maybe",
};

TEST(enum_names, continues_by_value)
{
    static_assert(continues_type_names[continues_type::off] == "off");
    static_assert(continues_type_names[continues_type::on] == "on");
    static_assert(continues_type_names[continues_type::maybe] == "maybe");

    static_assert(continues_type_names.at(continues_type::off) == "off");
    static_assert(continues_type_names.at(continues_type::on) == "on");
    static_assert(continues_type_names.at(continues_type::maybe) == "maybe");
    ASSERT_THROW(continues_type_name.at(continues_type{42}), std::out_of_range);

    static_assert(continues_type_names.at(continues_type::off, "default") == "off");
    static_assert(continues_type_names.at(continues_type::on, "default") == "on");
    static_assert(continues_type_names.at(continues_type::maybe, "default") == "maybe");
    static_assert(continues_type_name.at(continues_type{42}, "default") == "default");
}

TEST(enum_names, continues_by_name)
{
    static_assert(continues_type_names["off"] == continues_type::off);
    static_assert(continues_type_names["on"] == continues_type::on);
    static_assert(continues_type_names["maybe"] == continues_type::maybe);

    static_assert(continues_type_names.at("off") == continues_type::off);
    static_assert(continues_type_names.at("on") == continues_type::on);
    static_assert(continues_type_names.at("maybe") == continues_type::maybe);
    ASSERT_THROW(continues_type_names.at("foo"), std::out_of_range);

    static_assert(continues_type_names.at("off", continues_type{42}) == continues_type::off);
    static_assert(continues_type_names.at("on", continues_type{42}) == continues_type::on);
    static_assert(continues_type_names.at("maybe", continues_type{42}) == continues_type::maybe);
    static_assert(continues_type_names.at("foo", continues_type{42}) == continues_type{42});
}

TEST(enum_names, continues_by_name)
{
    static_assert(continues_type_names.contains("off") == true);
    static_assert(continues_type_names.contains("foo") == false);
    static_assert(continues_type_names.contains(continues_type::off) == true);
    static_assert(continues_type_names.contains(continues_type{42}) == false);
}

