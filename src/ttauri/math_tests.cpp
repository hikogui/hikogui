// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/math.hpp"
#include "ttauri/required.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;

TEST(math, almost_equal)
{
    static_assert(almost_equal(0.0, 0.0) == true);
    static_assert(almost_equal(0.0, 1.4012984643e-45f) == true);
    static_assert(almost_equal(0.0, 6e-45f) == true);
    static_assert(almost_equal(0.0, 7e-45f) == false);
    static_assert(almost_equal(1.4012984643e-45f, 0.0) == true);
    static_assert(almost_equal(6e-45f, 0.0) == true);
    static_assert(almost_equal(7e-45f, 0.0) == false);

    static_assert(almost_equal(-0.0, -0.0) == true);
    static_assert(almost_equal(-0.0, -1.4012984643e-45f) == true);
    static_assert(almost_equal(-0.0, -6e-45f) == true);
    static_assert(almost_equal(-0.0, -7e-45f) == false);
    static_assert(almost_equal(-1.4012984643e-45f, -0.0) == true);
    static_assert(almost_equal(-6e-45f, -0.0) == true);
    static_assert(almost_equal(-7e-45f, -0.0) == false);

    static_assert(almost_equal(-0.0, 0.0) == true);
    static_assert(almost_equal(-0.0, 1.4012984643e-45f) == true);
    static_assert(almost_equal(-0.0, 6e-45f) == true);
    static_assert(almost_equal(-0.0, 7e-45f) == false);
    static_assert(almost_equal(-1.4012984643e-45f, 0.0) == true);
    static_assert(almost_equal(-6e-45f, 0.0) == true);
    static_assert(almost_equal(-7e-45f, 0.0) == false);

    static_assert(almost_equal(0.0, -0.0) == true);
    static_assert(almost_equal(0.0, -1.4012984643e-45f) == true);
    static_assert(almost_equal(0.0, -6e-45f) == true);
    static_assert(almost_equal(0.0, -7e-45f) == false);
    static_assert(almost_equal(1.4012984643e-45f, -0.0) == true);
    static_assert(almost_equal(6e-45f, -0.0) == true);
    static_assert(almost_equal(7e-45f, -0.0) == false);

    static_assert(almost_equal(1.4012984643e-45f, 1.4012984643e-45f) == true);
    static_assert(almost_equal(1.4012984643e-45f, 7e-45f) == true);
    static_assert(almost_equal(1.4012984643e-45f, 8e-45f) == false);

    static_assert(almost_equal(-1.4012984643e-45f, -1.4012984643e-45f) == true);
    static_assert(almost_equal(-1.4012984643e-45f, -7e-45f) == true);
    static_assert(almost_equal(-1.4012984643e-45f, -8e-45f) == false);

    static_assert(almost_equal(1.4012984643e-45f, -1.4012984643e-45f) == true);
    static_assert(almost_equal(1.4012984643e-45f, -4e-45f) == true);
    static_assert(almost_equal(1.4012984643e-45f, -5e-45f) == false);

    static_assert(almost_equal(-1.4012984643e-45f, 1.4012984643e-45f) == true);
    static_assert(almost_equal(-1.4012984643e-45f, 4e-45f) == true);
    static_assert(almost_equal(-1.4012984643e-45f, 5e-45f) == false);

    static_assert(almost_equal(1.0, 1.0) == true);
    static_assert(almost_equal(1.0, 1.0000005f) == true);
    static_assert(almost_equal(1.0, 1.0000006f) == false);
    static_assert(almost_equal(1.0000005f, 1.0) == true);
    static_assert(almost_equal(1.0000006f, 1.0) == false);

    static_assert(almost_equal(-1.0, -1.0) == true);
    static_assert(almost_equal(-1.0, -1.0000005f) == true);
    static_assert(almost_equal(-1.0, -1.0000006f) == false);
    static_assert(almost_equal(-1.0000005f, -1.0) == true);
    static_assert(almost_equal(-1.0000006f, -1.0) == false);

    static_assert(almost_equal(1.0, -1.0) == false);
    static_assert(almost_equal(1.0, -1.0000005f) == false);
    static_assert(almost_equal(1.0, -1.0000006f) == false);
    static_assert(almost_equal(1.0000005f, -1.0) == false);
    static_assert(almost_equal(1.0000006f, -1.0) == false);

    static_assert(almost_equal(-1.0, 1.0) == false);
    static_assert(almost_equal(-1.0, 1.0000005f) == false);
    static_assert(almost_equal(-1.0, 1.0000006f) == false);
    static_assert(almost_equal(-1.0000005f, 1.0) == false);
    static_assert(almost_equal(-1.0000006f, 1.0) == false);
}