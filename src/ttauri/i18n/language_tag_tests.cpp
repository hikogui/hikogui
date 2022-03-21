// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "language_tag.hpp"
#include <format>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>

TEST(language_tag, construct_from_string)
{
    auto nl = tt::language_tag{"nl"};
    auto nl_NL = tt::language_tag{"nl-NL"};
    auto nl_BE = tt::language_tag{"nl-BE"};

    ASSERT_EQ(nl.to_string(), std::string("nl-Latn-NL"));
    ASSERT_EQ(nl_NL.to_string(), std::string("nl-Latn-NL"));
    ASSERT_EQ(nl_BE.to_string(), std::string("nl-Latn-BE"));
}
