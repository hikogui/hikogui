// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "xorshift128p.hpp"
#include "../SIMD/simd_test_utility.hpp"
#include <iostream>
#include <string>
#include <limits>

using namespace std;
using namespace hi;

TEST(xorshift128p, compare_64_and_128_bits)
{
    auto r1 = hi::xorshift128p();
    // Make a copy with the same seed.
    auto r2 = r1;

    for (auto i = 0; i != 100000; ++i) {
        auto expected = u64x2{};
        expected[0] = r1.next<uint64_t>();
        expected[1] = r1.next<uint64_t>();

        auto result = r2.next<u64x2>();
        HI_ASSERT_SIMD_EQ(result, expected);
    }
}
