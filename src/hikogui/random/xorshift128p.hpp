// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "seed.hpp"
#include "../SIMD/SIMD.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <random>

hi_export_module(hikogui.random.xorshift128p);

hi_export namespace hi::inline v1 {

/** xorshift128+
 */
class xorshift128p {
public:
    constexpr xorshift128p(xorshift128p const &) noexcept = default;
    constexpr xorshift128p(xorshift128p &&) noexcept = default;
    constexpr xorshift128p &operator=(xorshift128p const &) noexcept = default;
    constexpr xorshift128p &operator=(xorshift128p &&) noexcept = default;

    [[nodiscard]] constexpr explicit xorshift128p(u64x2 new_state) noexcept : _state(new_state) {}

    [[nodiscard]] xorshift128p() noexcept : _state{}
    {
        while (_state.x() == 0 or _state.y() == 0) {
            _state = seed<u64x2>{}();
        }
    }

    template<typename T>
    [[nodiscard]] T next() noexcept;

    /** Get the next 64 bit of random value.
     */
    template<>
    [[nodiscard]] uint64_t next() noexcept
    {
        auto s = _state[0];
        hilet t = _state[1];

        s ^= s << 23; // a
        s ^= s >> 17; // b
        s ^= t ^ (t >> 26); // c

        _state[0] = t;
        _state[1] = s;
        return s + t;
    }

private:
    u64x2 _state;
};

} // namespace hi::inline v1
