// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "spreadsheet_address.hpp"
#include <hikotest/hikotest.hpp>
#include <cstddef>

TEST_SUITE(spreadsheet_address) {

TEST_CASE(parse_absolute_spreadsheet_address)
{
    REQUIRE(hi::parse_spreadsheet_address("A1") == std::pair(size_t{0}, size_t{0}));
    REQUIRE(hi::parse_spreadsheet_address("A9") == std::pair(size_t{0}, size_t{8}));
    REQUIRE(hi::parse_spreadsheet_address("A09") == std::pair(size_t{0}, size_t{8}));
    REQUIRE(hi::parse_spreadsheet_address("A10") == std::pair(size_t{0}, size_t{9}));

    REQUIRE(hi::parse_spreadsheet_address("a1") == std::pair(size_t{0}, size_t{0}));
    REQUIRE(hi::parse_spreadsheet_address("a9") == std::pair(size_t{0}, size_t{8}));
    REQUIRE(hi::parse_spreadsheet_address("a09") == std::pair(size_t{0}, size_t{8}));
    REQUIRE(hi::parse_spreadsheet_address("a10") == std::pair(size_t{0}, size_t{9}));

    REQUIRE(hi::parse_spreadsheet_address("B1") == std::pair(size_t{1}, size_t{0}));
    REQUIRE(hi::parse_spreadsheet_address("B9") == std::pair(size_t{1}, size_t{8}));
    REQUIRE(hi::parse_spreadsheet_address("B09") == std::pair(size_t{1}, size_t{8}));
    REQUIRE(hi::parse_spreadsheet_address("B10") == std::pair(size_t{1}, size_t{9}));

    REQUIRE(hi::parse_spreadsheet_address("Z1") == std::pair(size_t{25}, size_t{0}));
    REQUIRE(hi::parse_spreadsheet_address("Z9") == std::pair(size_t{25}, size_t{8}));
    REQUIRE(hi::parse_spreadsheet_address("Z09") == std::pair(size_t{25}, size_t{8}));
    REQUIRE(hi::parse_spreadsheet_address("Z10") == std::pair(size_t{25}, size_t{9}));

    REQUIRE(hi::parse_spreadsheet_address("AA1") == std::pair(size_t{26}, size_t{0}));
    REQUIRE(hi::parse_spreadsheet_address("AA9") == std::pair(size_t{26}, size_t{8}));
    REQUIRE(hi::parse_spreadsheet_address("AA09") == std::pair(size_t{26}, size_t{8}));
    REQUIRE(hi::parse_spreadsheet_address("AA10") == std::pair(size_t{26}, size_t{9}));
}

};