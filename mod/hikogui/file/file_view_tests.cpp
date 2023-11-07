// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file_view.hpp"
#include "../path/path.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>



using namespace std;
using namespace hi;

TEST(file_view, read)
{
    hilet view = file_view{library_source_dir() / "tests" / "data" / "file_view.txt"};

    ASSERT_EQ(as_string_view(view), "The quick brown fox jumps over the lazy dog.");
}
