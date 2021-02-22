// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/text/language_tag.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>

TEST(language_tag, format)
{
    auto tag = tt::language_tag{"en-GB"};

    ASSERT_EQ(to_string(tag), std::string("en-GB"));

    auto s = std::stringstream{};
    s << tag;
    ASSERT_EQ(s.str(), std::string("en-GB"));

    ASSERT_EQ(fmt::format("{}", tag), std::string("en-GB"));
}
