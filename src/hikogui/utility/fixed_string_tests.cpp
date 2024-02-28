// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "fixed_string.hpp"
#include <hikotest/hikotest.hpp>
#include <string>

TEST_SUITE(fixed_string) {

TEST_CASE(from_string_literal)
{
    constexpr auto s = hi::fixed_string{"Hello World"};
    REQUIRE(s == std::string("Hello World"));
    REQUIRE(s.size() == 11);
}

#if HI_COMPILER == HI_CC_MSVC
TEST_CASE(to_fixed_string)
{
    static_assert(hi_to_fixed_string(std::string{"hello"}) == hi::fixed_string{"hello"});
}
#endif

};
