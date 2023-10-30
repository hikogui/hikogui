// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <random>

export module hikogui_random_xorshift128p;
import hikogui_SIMD;
import hikogui_random_seed;
import hikogui_utility;

export namespace hi::inline v1 {

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

    /** Get next 128 bit of random value.
     *
     * The algorithm is based around `next64()`, it was modified to do two
     * consecutive iterations and then merging those using sse instructions.
     */
    template<>
    u64x2 next() noexcept
    {
        // scalar: uint64_t x = _state[0];
        // scalar: uint64_t y = y_ = _state[1];
        auto s = _state;
        auto t = s.yx();

        // scalar: x ^= x << 23;
        // scalar: y ^= y << 23;
        s ^= (s << 23);

        // scalar: x ^= x >> 17;
        // scalar: y ^= y >> 17;
        s ^= (s >> 17);

        // scalar: x ^= y_ ^ (y_ >> 26)
        hilet tmp = s ^ t ^ (t >> 26);

        // scalar: auto x_ = x;
        // scalar: t.y() = tmp.x();
        t = insert<0, 1>(t, tmp);

        // scalar: y ^= x_ ^ (x_ >> 26);
        s ^= t ^ (t >> 26);

        // scalar: state[0] = x
        // scalar: state[1] = y
        _state = s;

        // scalar: return {x + y_, y + x_}
        return s + t;
    }

    template<>
    [[nodiscard]] u32x4 next() noexcept
    {
        return u32x4::cast_from(next<u64x2>());
    }

    template<>
    [[nodiscard]] i32x4 next() noexcept
    {
        return i32x4::cast_from(next<u64x2>());
    }

    template<>
    [[nodiscard]] i16x8 next() noexcept
    {
        return i16x8::cast_from(next<u64x2>());
    }

private:
    u64x2 _state;
};

} // namespace hi::inline v1
