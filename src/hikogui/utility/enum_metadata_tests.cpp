// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "enum_metadata.hpp"
#include <gtest/gtest.h>

enum class continues_type {
    off,
    on,
    maybe
};

constexpr auto continues_type_names = hi::enum_metadata{
    continues_type::on, "on",
    continues_type::off, "off",
    continues_type::maybe, "maybe",
};

TEST(enum_metadata, continues_constants)
{
    static_assert(continues_type_names.count == 3);
    static_assert(continues_type_names.minimum() == continues_type::off);
    static_assert(continues_type_names.maximum() == continues_type::maybe);
    static_assert(continues_type_names.values_are_continues == true);
}

TEST(enum_metadata, continues_by_value)
{
    static_assert(continues_type_names[continues_type::off] == "off");
    static_assert(continues_type_names[continues_type::on] == "on");
    static_assert(continues_type_names[continues_type::maybe] == "maybe");

    static_assert(continues_type_names.at(continues_type::off) == "off");
    static_assert(continues_type_names.at(continues_type::on) == "on");
    static_assert(continues_type_names.at(continues_type::maybe) == "maybe");
    ASSERT_THROW((void)continues_type_names.at(continues_type{42}), std::out_of_range);

    static_assert(continues_type_names.at(continues_type::off, "default") == "off");
    static_assert(continues_type_names.at(continues_type::on, "default") == "on");
    static_assert(continues_type_names.at(continues_type::maybe, "default") == "maybe");
    static_assert(continues_type_names.at(continues_type{42}, "default") == "default");
}

TEST(enum_metadata, continues_by_name)
{
    static_assert(continues_type_names["off"] == continues_type::off);
    static_assert(continues_type_names["on"] == continues_type::on);
    static_assert(continues_type_names["maybe"] == continues_type::maybe);

    static_assert(continues_type_names.at("off") == continues_type::off);
    static_assert(continues_type_names.at("on") == continues_type::on);
    static_assert(continues_type_names.at("maybe") == continues_type::maybe);
    ASSERT_THROW((void)continues_type_names.at("foo"), std::out_of_range);

    static_assert(continues_type_names.at("off", continues_type{42}) == continues_type::off);
    static_assert(continues_type_names.at("on", continues_type{42}) == continues_type::on);
    static_assert(continues_type_names.at("maybe", continues_type{42}) == continues_type::maybe);
    static_assert(continues_type_names.at("foo", continues_type{42}) == continues_type{42});
}

TEST(enum_metadata, continues_contains)
{
    static_assert(continues_type_names.contains("off") == true);
    static_assert(continues_type_names.contains("foo") == false);
    static_assert(continues_type_names.contains(continues_type::off) == true);
    static_assert(continues_type_names.contains(continues_type{42}) == false);
}

enum class offset_type {
    maybe = -1,
    off,
    on,
};

constexpr auto offset_type_names = hi::enum_metadata{
    offset_type::on,
    "on",
    offset_type::off,
    "off",
    offset_type::maybe,
    "maybe",
};

TEST(enum_metadata, offset_constants)
{
    static_assert(offset_type_names.count == 3);
    static_assert(offset_type_names.minimum() == offset_type::maybe);
    static_assert(offset_type_names.maximum() == offset_type::on);
    static_assert(offset_type_names.values_are_continues == true);
}

TEST(enum_metadata, offset_by_value)
{
    static_assert(offset_type_names[offset_type::off] == "off");
    static_assert(offset_type_names[offset_type::on] == "on");
    static_assert(offset_type_names[offset_type::maybe] == "maybe");

    static_assert(offset_type_names.at(offset_type::off) == "off");
    static_assert(offset_type_names.at(offset_type::on) == "on");
    static_assert(offset_type_names.at(offset_type::maybe) == "maybe");
    ASSERT_THROW((void)offset_type_names.at(offset_type{42}), std::out_of_range);

    static_assert(offset_type_names.at(offset_type::off, "default") == "off");
    static_assert(offset_type_names.at(offset_type::on, "default") == "on");
    static_assert(offset_type_names.at(offset_type::maybe, "default") == "maybe");
    static_assert(offset_type_names.at(offset_type{42}, "default") == "default");
}

TEST(enum_metadata, offset_by_name)
{
    static_assert(offset_type_names["off"] == offset_type::off);
    static_assert(offset_type_names["on"] == offset_type::on);
    static_assert(offset_type_names["maybe"] == offset_type::maybe);

    static_assert(offset_type_names.at("off") == offset_type::off);
    static_assert(offset_type_names.at("on") == offset_type::on);
    static_assert(offset_type_names.at("maybe") == offset_type::maybe);
    ASSERT_THROW((void)offset_type_names.at("foo"), std::out_of_range);

    static_assert(offset_type_names.at("off", offset_type{42}) == offset_type::off);
    static_assert(offset_type_names.at("on", offset_type{42}) == offset_type::on);
    static_assert(offset_type_names.at("maybe", offset_type{42}) == offset_type::maybe);
    static_assert(offset_type_names.at("foo", offset_type{42}) == offset_type{42});
}

TEST(enum_metadata, offset_contains)
{
    static_assert(offset_type_names.contains("off") == true);
    static_assert(offset_type_names.contains("foo") == false);
    static_assert(offset_type_names.contains(offset_type::off) == true);
    static_assert(offset_type_names.contains(offset_type{42}) == false);
}


enum class discontinues_type {
    maybe = -2,
    off = 0,
    on = 1,
};

constexpr auto discontinues_type_names = hi::enum_metadata{
    discontinues_type::on,
    "on",
    discontinues_type::off,
    "off",
    discontinues_type::maybe,
    "maybe",
};

TEST(enum_metadata, discontinues_constants)
{
    static_assert(discontinues_type_names.count == 3);
    static_assert(discontinues_type_names.minimum() == discontinues_type::maybe);
    static_assert(discontinues_type_names.maximum() == discontinues_type::on);
    static_assert(discontinues_type_names.values_are_continues == false);
}

TEST(enum_metadata, discontinues_by_value)
{
    static_assert(discontinues_type_names[discontinues_type::off] == "off");
    static_assert(discontinues_type_names[discontinues_type::on] == "on");
    static_assert(discontinues_type_names[discontinues_type::maybe] == "maybe");

    static_assert(discontinues_type_names.at(discontinues_type::off) == "off");
    static_assert(discontinues_type_names.at(discontinues_type::on) == "on");
    static_assert(discontinues_type_names.at(discontinues_type::maybe) == "maybe");
    ASSERT_THROW((void)discontinues_type_names.at(discontinues_type{42}), std::out_of_range);

    static_assert(discontinues_type_names.at(discontinues_type::off, "default") == "off");
    static_assert(discontinues_type_names.at(discontinues_type::on, "default") == "on");
    static_assert(discontinues_type_names.at(discontinues_type::maybe, "default") == "maybe");
    static_assert(discontinues_type_names.at(discontinues_type{42}, "default") == "default");
}

TEST(enum_metadata, discontinues_by_name)
{
    static_assert(discontinues_type_names["off"] == discontinues_type::off);
    static_assert(discontinues_type_names["on"] == discontinues_type::on);
    static_assert(discontinues_type_names["maybe"] == discontinues_type::maybe);

    static_assert(discontinues_type_names.at("off") == discontinues_type::off);
    static_assert(discontinues_type_names.at("on") == discontinues_type::on);
    static_assert(discontinues_type_names.at("maybe") == discontinues_type::maybe);
    ASSERT_THROW((void)discontinues_type_names.at("foo"), std::out_of_range);

    static_assert(discontinues_type_names.at("off", discontinues_type{42}) == discontinues_type::off);
    static_assert(discontinues_type_names.at("on", discontinues_type{42}) == discontinues_type::on);
    static_assert(discontinues_type_names.at("maybe", discontinues_type{42}) == discontinues_type::maybe);
    static_assert(discontinues_type_names.at("foo", discontinues_type{42}) == discontinues_type{42});
}

TEST(enum_metadata, discontinues_contains)
{
    static_assert(discontinues_type_names.contains("off") == true);
    static_assert(discontinues_type_names.contains("foo") == false);
    static_assert(discontinues_type_names.contains(discontinues_type::off) == true);
    static_assert(discontinues_type_names.contains(discontinues_type{42}) == false);
}