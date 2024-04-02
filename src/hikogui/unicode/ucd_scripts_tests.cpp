// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ucd_scripts.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(ucd_scripts) {

TEST_CASE(to_unicode_script)
{
    REQUIRE(hi::iso_15924(215) == hi::unicode_script::Latin);
    REQUIRE(hi::iso_15924(460) == hi::unicode_script::Yi);
}

TEST_CASE(to_iso_15924)
{
    REQUIRE(hi::unicode_script::Latin == hi::iso_15924(215));
    REQUIRE(hi::unicode_script::Yi == hi::iso_15924(460));
}

};
