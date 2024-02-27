// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "font_char_map.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(font_char_map) {

TEST_CASE(add_and_search)
{
    auto cm = hi::font_char_map{};

    cm.add(U'a', U'z', 100);
    cm.add(U'5', U'6', 305);
    cm.add(U'0', U'3', 200);
    cm.add(U'4', U'4', 204);
    // '7' is missing.
    cm.add(U'8', U'9', 208);

    cm.prepare();

    REQUIRE(cm.find(U'a') == 100);
    REQUIRE(cm.find(U'b') == 101);
    REQUIRE(cm.find(U'z') == 125);
    REQUIRE(cm.find(U'0') == 200);
    REQUIRE(cm.find(U'1') == 201);
    REQUIRE(cm.find(U'2') == 202);
    REQUIRE(cm.find(U'3') == 203);
    REQUIRE(cm.find(U'4') == 204);
    REQUIRE(cm.find(U'5') == 305);
    REQUIRE(cm.find(U'6') == 306);
    REQUIRE(cm.find(U'7') == 0xffff);
    REQUIRE(cm.find(U'8') == 208);
    REQUIRE(cm.find(U'9') == 209);
}

};
