// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gstring.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>

TEST(gstring, from_utf8)
{
    auto test = hi::to_gstring("This is a \xd7\x9c\xd6\xb0\xd7\x9e\xd6\xb7\xd7\xaa\xd6\xb5\xd7\x92.\nAnd another sentence. One more:");
    //ASSERT_EQ(test.size(), 30);
    ASSERT_NE(test[10].starter(), 0);
}
