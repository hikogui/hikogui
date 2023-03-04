// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_script.hpp"
#include <gtest/gtest.h>

TEST(unicode_script, to_unicode_script)
{
    ASSERT_EQ(hi::to_unicode_script(hi::iso_15924(215)), hi::unicode_script::Latin);
    ASSERT_EQ(hi::to_unicode_script(hi::iso_15924(460)), hi::unicode_script::Yi);
}

TEST(unicode_script, to_iso_15924)
{
    ASSERT_EQ(hi::to_iso_15924(hi::unicode_script::Latin).number(), hi::iso_15924(215));
    ASSERT_EQ(hi::to_iso_15924(hi::unicode_script::Yi).number(), hi::iso_15924(460));
}
