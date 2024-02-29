// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "lookahead_iterator.hpp"
#include <hikotest/hikotest.hpp>
#include <vector>
#include <iterator>

TEST_SUITE(lookahead_iterator) {

TEST_CASE(iterate)
{
    auto values = std::vector{'a', 'b', 'c', 'd', 'e', 'f'};

    auto it = hi::make_lookahead_iterator<4>(values.begin(), values.end());

    REQUIRE(it.size() == 4);
    REQUIRE(not it.empty());
    REQUIRE(it != std::default_sentinel);
    REQUIRE(*it == 'a');
    REQUIRE(it[0] == 'a');
    REQUIRE(it[1] == 'b');
    REQUIRE(it[2] == 'c');
    REQUIRE(it[3] == 'd');

    ++it;
    REQUIRE(it.size() == 4);
    REQUIRE(not it.empty());
    REQUIRE(it != std::default_sentinel);
    REQUIRE(*it == 'b');
    REQUIRE(it[0] == 'b');
    REQUIRE(it[1] == 'c');
    REQUIRE(it[2] == 'd');
    REQUIRE(it[3] == 'e');

    ++it;
    REQUIRE(it.size() == 4);
    REQUIRE(not it.empty());
    REQUIRE(it != std::default_sentinel);
    REQUIRE(*it == 'c');
    REQUIRE(it[0] == 'c');
    REQUIRE(it[1] == 'd');
    REQUIRE(it[2] == 'e');
    REQUIRE(it[3] == 'f');

    ++it;
    REQUIRE(it.size() == 3);
    REQUIRE(not it.empty());
    REQUIRE(it != std::default_sentinel);
    REQUIRE(*it == 'd');
    REQUIRE(it[0] == 'd');
    REQUIRE(it[1] == 'e');
    REQUIRE(it[2] == 'f');

    ++it;
    REQUIRE(it.size() == 2);
    REQUIRE(not it.empty());
    REQUIRE(it != std::default_sentinel);
    REQUIRE(*it == 'e');
    REQUIRE(it[0] == 'e');
    REQUIRE(it[1] == 'f');

    ++it;
    REQUIRE(it.size() == 1);
    REQUIRE(not it.empty());
    REQUIRE(it != std::default_sentinel);
    REQUIRE(*it == 'f');
    REQUIRE(it[0] == 'f');

    ++it;
    REQUIRE(it.size() == 0);
    REQUIRE(it.empty());
    REQUIRE(it == std::default_sentinel);
}

}; // TEST_SUITE(lookahead_iterator)
