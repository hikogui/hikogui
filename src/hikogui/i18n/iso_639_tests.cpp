// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/i18n/iso_639.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

TEST(iso_639, parse)
{
    ASSERT_EQ(tt::iso_639("nl").code(), "nl");
    ASSERT_EQ(tt::iso_639("NL").code(), "nl");
    ASSERT_EQ(tt::iso_639("Nl").code(), "nl");
    ASSERT_EQ(tt::iso_639("nL").code(), "nl");

    ASSERT_EQ(tt::iso_639("foo").code(), "foo");

    ASSERT_THROW(tt::iso_639("n"), tt::parse_error);
    ASSERT_THROW(tt::iso_639("food"), tt::parse_error);

    static_assert(tt::iso_639("nl").code() == "nl");
    static_assert(tt::iso_639("NL").code() == "nl");
    static_assert(tt::iso_639("Nl").code() == "nl");
    static_assert(tt::iso_639("nL").code() == "nl");
    static_assert(tt::iso_639("foo").code() == "foo");
}

TEST(iso_639, size)
{
    ASSERT_EQ(tt::iso_639().size(), 0);
    ASSERT_EQ(tt::iso_639().empty(), true);
    ASSERT_EQ(tt::iso_639("nl").size(), 2);
    ASSERT_EQ(tt::iso_639("nl").empty(), false);
    ASSERT_EQ(tt::iso_639("foo").size(), 3);
    ASSERT_EQ(tt::iso_639("foo").empty(), false);

    static_assert(tt::iso_639().size() == 0);
    static_assert(tt::iso_639().empty() == true);
    static_assert(tt::iso_639("nl").size() == 2);
    static_assert(tt::iso_639("nl").empty() == false);
    static_assert(tt::iso_639("foo").size() == 3);
    static_assert(tt::iso_639("foo").empty() == false);
}

TEST(iso_639, hash)
{
    ASSERT_EQ(std::hash<tt::iso_639>{}(tt::iso_639()), std::hash<tt::iso_639>{}(tt::iso_639()));
    ASSERT_NE(std::hash<tt::iso_639>{}(tt::iso_639()), std::hash<tt::iso_639>{}(tt::iso_639("nl")));
    ASSERT_EQ(std::hash<tt::iso_639>{}(tt::iso_639("nl")), std::hash<tt::iso_639>{}(tt::iso_639("nl")));
    ASSERT_NE(std::hash<tt::iso_639>{}(tt::iso_639("nl")), std::hash<tt::iso_639>{}(tt::iso_639("be")));
}
