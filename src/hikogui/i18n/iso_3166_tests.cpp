// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "iso_3166.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(iso_3166_suite)
{

TEST_CASE(parse_test)
{
    REQUIRE(hi::iso_3166("NL").number() == 528);
    REQUIRE(hi::iso_3166("Nl").number() == 528);
    REQUIRE(hi::iso_3166("nl").number() == 528);
    REQUIRE(hi::iso_3166("NLD").number() == 528);
    REQUIRE(hi::iso_3166("Nld").number() == 528);
    REQUIRE(hi::iso_3166("nld").number() == 528);
    REQUIRE(hi::iso_3166("528").number() == 528);

    REQUIRE(hi::iso_3166("BE").number() == 56);
    REQUIRE(hi::iso_3166("Be").number() == 56);
    REQUIRE(hi::iso_3166("be").number() == 56);
    REQUIRE(hi::iso_3166("BEL").number() == 56);
    REQUIRE(hi::iso_3166("Bel").number() == 56);
    REQUIRE(hi::iso_3166("bel").number() == 56);
    REQUIRE(hi::iso_3166("056").number() == 56);
    REQUIRE(hi::iso_3166("56").number() == 56);

    REQUIRE_THROWS(hi::iso_3166(""), hi::parse_error);
    REQUIRE_THROWS(hi::iso_3166("x"), hi::parse_error);
    REQUIRE_THROWS(hi::iso_3166("xx"), hi::parse_error);
    REQUIRE_THROWS(hi::iso_3166("xxx"), hi::parse_error);
    REQUIRE_THROWS(hi::iso_3166("xxxx"), hi::parse_error);

    REQUIRE(hi::iso_3166("0").number() == 0);
    REQUIRE(hi::iso_3166("0").empty());
    REQUIRE(hi::iso_3166("1").number() == 1);
    REQUIRE(hi::iso_3166("01").number() == 1);
    REQUIRE(hi::iso_3166("10").number() == 10);
    REQUIRE(hi::iso_3166("010").number() == 10);
    REQUIRE(hi::iso_3166("100").number() == 100);
    REQUIRE(hi::iso_3166("0100").number() == 100);
    REQUIRE(hi::iso_3166("999").number() == 999);
    REQUIRE_THROWS(hi::iso_3166("1000"), hi::parse_error);
    REQUIRE_THROWS(hi::iso_3166("-1"), hi::parse_error);
}

TEST_CASE(code2_test)
{
    REQUIRE(hi::iso_3166(528).code2() == "NL");
    REQUIRE(hi::iso_3166(56).code2() == "BE");
    REQUIRE(hi::iso_3166(999).code2() == "ZZ");
}

TEST_CASE(code3_test)
{
    REQUIRE(hi::iso_3166(528).code3() == "NLD");
    REQUIRE(hi::iso_3166(56).code3() == "BEL");
    REQUIRE(hi::iso_3166(999).code3() == "ZZZ");
}

};
