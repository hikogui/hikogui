// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

import hikogui_utility;
#include "file_view.hpp"
#include "../utility/module.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(file_view, read)
{
    hilet view = file_view{"file_view.txt"};

    ASSERT_EQ(as_string_view(view), "The quick brown fox jumps over the lazy dog.");
}
