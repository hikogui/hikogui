// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "not_null.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(not_null) {

struct A {};

struct B : A {}; 


TEST_CASE(make_shared_implicit_cast)
{
    auto b = std::make_shared<B>();
    auto a_copy = hi::not_null<std::shared_ptr<A>>{b};
    auto a_move = hi::not_null<std::shared_ptr<A>>{std::move(b)};
}

TEST_CASE(make_shared_not_null_implicit_cast)
{
    auto b = hi::make_shared_not_null<B>();
    auto a_copy = hi::not_null<std::shared_ptr<A>>{b};
    auto a_move = hi::not_null<std::shared_ptr<A>>{std::move(b)};
}

};
