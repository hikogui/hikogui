// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file_view.hpp"
#include "../utility.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(file_view, read)
{
    hilet view = file_view("file:file_view.txt");

    hilet *test = reinterpret_cast<char const *>(view.span().data());
    ASSERT_TRUE(strncmp(test, "The quick brown", 15) == 0);
}
