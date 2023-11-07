// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "not_null.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>

namespace not_null_tests {

struct A {};

struct B : A {}; 

}

TEST(not_null, make_shared_implicit_cast)
{
    auto b = std::make_shared<not_null_tests::B>();
    auto a_copy = hi::not_null<std::shared_ptr<not_null_tests::A>>{b};
    auto a_move = hi::not_null<std::shared_ptr<not_null_tests::A>>{std::move(b)};
}

TEST(not_null, make_shared_not_null_implicit_cast)
{
    auto b = hi::make_shared_not_null<not_null_tests::B>();
    auto a_copy = hi::not_null<std::shared_ptr<not_null_tests::A>>{b};
    auto a_move = hi::not_null<std::shared_ptr<not_null_tests::A>>{std::move(b)};
}
