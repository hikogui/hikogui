// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file_view.hpp"
#include "../path/path.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(file_view) {

TEST_CASE(read)
{
    auto const view = hi::file_view{hi::library_source_dir() / "tests" / "data" / "file_view.txt"};

    REQUIRE(as_string_view(view) == "The quick brown fox jumps over the lazy dog.");
}

};
