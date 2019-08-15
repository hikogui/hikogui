// All rights reserved.

#include "any_repr.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace TTauri;

TEST(AnyRepr, Default) {
    ASSERT_EQ(any_repr(int{42}), "42"s);
    ASSERT_EQ(any_repr("foo"s), "\"foo\""s);
}
