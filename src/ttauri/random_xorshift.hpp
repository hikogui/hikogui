// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "geometry/numeric_array.hpp"

namespace tt {

/** xorshift128aes with 128 bit state and 128 bit result.
 *
 * This xorshift is implemented as two independent xorshift64,
 * followed by two rounds of aesenc which fully shuffles the 128 bits.
 * 
 * We can implement the following pairs of xorshift64 algorithms by a
 * combination of shift and SSE 32 bit permutes.
 *  - (1, 23, 14), (33, 23, 14) *
 *  - (3, 29, 49), (3, 61, 17) +
 *  - (4, 7, 19), (36, 7, 19) *
 *  - (4, 9, 13), (4, 41, 45) +
 *  - (11, 5, 43), (43, 5, 11) +
 *  - (11, 25, 48), (43, 25, 16) +
 *  - (14, 15, 19), (46, 15, 19) *
 *  - (14, 23, 33), (14, 23, 1) *
 *  - (16, 25, 43), (48, 25, 11) +
 *  - (17, 47, 29), (49, 15, 61)
 *  - (19, 7, 36), (19, 7, 4) *
 *  - (19, 15, 46), (19, 15, 14) *
 *  - (25, 11, 57), (57, 11, 25) +
 *  - (49, 29, 3), (17, 61, 3) +
 *  - (13, 9, 4), (45, 41, 4) +
 *  - (29, 47, 17), (61, 15, 49)
 *
 * The implementation below uses the pair: (19, 7, 36), (19, 7, 4)
 * This pair has low values, which means more bits get mixed in each iteration
 * and two of the values are equal, meaning we only need a single SSE permute.
*/
class random_xorshifts128aes {
public:
    constexpr random_xorshifts_u32x4(random_xorshifts_u32x4 &const) noexcept = default;
    constexpr random_xorshifts_u32x4(random_xorshifts_u32x4 &&) noexcept = default;
    constexpr random_xorshifts_u32x4 &operator=(random_xorshifts_u32x4 &const) noexcept = default;
    constexpr random_xorshifts_u32x4 &operator=(random_xorshifts_u32x4 &&) noexcept = default;

    [[nodiscard]] constexpr u64x2 operator() noexcept
    {
        auto x = _state;
        auto zero = u64x2{};

        // One round only permutes one 64 bit lane, and swaps the two lanes.
        // So we need to use another round after this. We mix the aes rounds
        // with the xorshift since an aes round takes 4 cycles (Skylake) of latency.
        auto r = aesenc_round(x, zero);

        // Execute a single iteration of two xorshift64 algorithms in parallel.
        x ^= x << 19;
        x ^= x >> 7;

        // The second AES is started 4 cycles (Skylake) before completing xorshift.
        // We can take the intermediate of the xorshift as the round key.
        r = aesenc_round(r, x);

        // Last 4 cycles.
        x ^= bit_cast<u64x2>(bit_cast<f32x4>(x << 4)._0xzw());
        _state = x;

        return r;
    }

private:
    u64x2 _state;
};


}