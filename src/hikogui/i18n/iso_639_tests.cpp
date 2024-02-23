// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "iso_639.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(iso_639_suite)
{

TEST_CASE(parse_test)
{
    REQUIRE(hi::iso_639("nl").code() == "nl");
    REQUIRE(hi::iso_639("NL").code() == "nl");
    REQUIRE(hi::iso_639("Nl").code() == "nl");
    REQUIRE(hi::iso_639("nL").code() == "nl");

    REQUIRE(hi::iso_639("foo").code() == "foo");

    REQUIRE_THROWS(hi::iso_639("n"), hi::parse_error);
    REQUIRE_THROWS(hi::iso_639("food"), hi::parse_error);
}

TEST_CASE(size_test)
{
    REQUIRE(hi::iso_639().size() == 0);
    REQUIRE(hi::iso_639().empty());
    REQUIRE(hi::iso_639("nl").size() == 2);
    REQUIRE(not hi::iso_639("nl").empty());
    REQUIRE(hi::iso_639("foo").size() == 3);
    REQUIRE(not hi::iso_639("foo").empty());
}

TEST_CASE(hash_test)
{
    REQUIRE(std::hash<hi::iso_639>{}(hi::iso_639()) == std::hash<hi::iso_639>{}(hi::iso_639()));
    REQUIRE(std::hash<hi::iso_639>{}(hi::iso_639()) != std::hash<hi::iso_639>{}(hi::iso_639("nl")));
    REQUIRE(std::hash<hi::iso_639>{}(hi::iso_639("nl")) == std::hash<hi::iso_639>{}(hi::iso_639("nl")));
    REQUIRE(std::hash<hi::iso_639>{}(hi::iso_639("nl")) != std::hash<hi::iso_639>{}(hi::iso_639("be")));
}

};
