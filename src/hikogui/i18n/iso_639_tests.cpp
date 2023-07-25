// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "iso_639.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

TEST(iso_639, parse)
{
    ASSERT_EQ(hi::iso_639("nl").code(), "nl");
    ASSERT_EQ(hi::iso_639("NL").code(), "nl");
    ASSERT_EQ(hi::iso_639("Nl").code(), "nl");
    ASSERT_EQ(hi::iso_639("nL").code(), "nl");

    ASSERT_EQ(hi::iso_639("foo").code(), "foo");

    ASSERT_THROW(hi::iso_639("n"), hi::parse_error);
    ASSERT_THROW(hi::iso_639("food"), hi::parse_error);

    static_assert(hi::iso_639("nl").code() == "nl");
    static_assert(hi::iso_639("NL").code() == "nl");
    static_assert(hi::iso_639("Nl").code() == "nl");
    static_assert(hi::iso_639("nL").code() == "nl");
    static_assert(hi::iso_639("foo").code() == "foo");
}

TEST(iso_639, size)
{
    ASSERT_EQ(hi::iso_639().size(), 0);
    ASSERT_EQ(hi::iso_639().empty(), true);
    ASSERT_EQ(hi::iso_639("nl").size(), 2);
    ASSERT_EQ(hi::iso_639("nl").empty(), false);
    ASSERT_EQ(hi::iso_639("foo").size(), 3);
    ASSERT_EQ(hi::iso_639("foo").empty(), false);

    static_assert(hi::iso_639().size() == 0);
    static_assert(hi::iso_639().empty() == true);
    static_assert(hi::iso_639("nl").size() == 2);
    static_assert(hi::iso_639("nl").empty() == false);
    static_assert(hi::iso_639("foo").size() == 3);
    static_assert(hi::iso_639("foo").empty() == false);
}

TEST(iso_639, hash)
{
    ASSERT_EQ(std::hash<hi::iso_639>{}(hi::iso_639()), std::hash<hi::iso_639>{}(hi::iso_639()));
    ASSERT_NE(std::hash<hi::iso_639>{}(hi::iso_639()), std::hash<hi::iso_639>{}(hi::iso_639("nl")));
    ASSERT_EQ(std::hash<hi::iso_639>{}(hi::iso_639("nl")), std::hash<hi::iso_639>{}(hi::iso_639("nl")));
    ASSERT_NE(std::hash<hi::iso_639>{}(hi::iso_639("nl")), std::hash<hi::iso_639>{}(hi::iso_639("be")));
}
