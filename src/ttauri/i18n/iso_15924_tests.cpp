// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/i18n/iso_15924.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

TEST(iso_15925, from_unicode)
{
    ASSERT_EQ(tt::iso_15924{"Latn"}, tt::iso_15924{215});
}
