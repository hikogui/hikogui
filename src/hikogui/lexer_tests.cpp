// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#include "lexer.hpp"
#include <gtest/gtest.h>


TEST(lexer, c_style)
{
    constexpr auto c_lexer = hi::lexer<hi::lexer_config::c_style()>;

    auto it = c_lexer.parse("1234, 1.23, \"hello\", foo;");
    ASSERT_EQ(*it, hi::lexer_token_type(hi::lexer_token_kind::integer_literal, "1234", 0));
    ++it;
    ASSERT_EQ(*it, hi::lexer_token_type(hi::lexer_token_kind::separator, ",", 4));
    ++it;
    ASSERT_EQ(*it, hi::lexer_token_type(hi::lexer_token_kind::float_literal, "1.23", 5));
    ++it;
    ASSERT_EQ(*it, hi::lexer_token_type(hi::lexer_token_kind::separator, ",", 10));
    ++it;
    ASSERT_EQ(*it, hi::lexer_token_type(hi::lexer_token_kind::dqstring_literal, "hello", 11));
    ++it;
    ASSERT_EQ(*it, hi::lexer_token_type(hi::lexer_token_kind::separator, ",", 19));
    ++it;
    ASSERT_EQ(*it, hi::lexer_token_type(hi::lexer_token_kind::identifier, "foo", 20));
    ++it;
    ASSERT_EQ(*it, hi::lexer_token_type(hi::lexer_token_kind::separator, ";", 24));
    ++it;
    ASSERT_EQ(it, std::default_sentinel);
}
