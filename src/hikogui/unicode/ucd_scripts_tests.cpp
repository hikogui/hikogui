// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ucd_scripts.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>

TEST(ucd_scripts, to_unicode_script)
{
    ASSERT_EQ(hi::iso_15924(215), hi::unicode_script::Latin);
    ASSERT_EQ(hi::iso_15924(460), hi::unicode_script::Yi);
}

TEST(ucd_scripts, to_iso_15924)
{
    ASSERT_EQ(hi::unicode_script::Latin, hi::iso_15924(215));
    ASSERT_EQ(hi::unicode_script::Yi, hi::iso_15924(460));
}
