// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "memory.hpp"
#include "math.hpp"
#include <cstddef>
#include <climits>
#include <concepts>
#include <array>

namespace hi::inline v1 {

/** An array of integers.
 *
 * The integers in the array are tightly packed without padding bits.
 *
 * @tparam BitsPerInteger Number of bits of each integer, between 0 and 57.
 * @tparam N The number of integers to store.
 */
template<size_t BitsPerInteger, size_t N>
class packed_int_array {
public:
    /** Number of bits of the unsigned integer.
     */
    constexpr static size_t bits_per_integer = BitsPerInteger;

    /** Number of bytes required to hold the number of bits.
     *
     * This needs extra 7 bits for alignment/adjustment.
     */
    constexpr static size_t store_size = ceil(bits_per_integer + CHAR_BIT - 1, size_t{CHAR_BIT}) / size_t{CHAR_BIT};
    static_assert(sizeof(unsigned long long) >= store_size);
    
    // clang-format off
    /** The value type of the unsigned integers that are stored in the array.
     *
     * The size of the unsigned integer type is automatically selected to be the smallest
     * type that can hold @a BitsPerInteger and can also be aligned/adjusted by up to 7 bits.
     */
    using value_type =
        std::conditional_t<sizeof(unsigned char) >= store_size, unsigned char,
        std::conditional_t<sizeof(unsigned short) >= store_size, unsigned short,
        std::conditional_t<sizeof(unsigned int) >= store_size, unsigned int,
        std::conditional_t<sizeof(unsigned long) >= store_size, unsigned long,
        unsigned long long>>>>;
    // clang-format on


    /** Constructor of the array.
     *
     * @param args A list of integers.
     */
    template<std::integral... Args>
    constexpr packed_int_array(Args... args) noexcept : _v(make_v(args...)) {}

    /** The number of integers stored in the array.
     */
    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return N;
    }

    /** Get the integer at an index.
     *
     * If possible this function will use `load()` to do an efficient unaligned
     * load from memory.
     *
     * @note It is undefined behavior if @i is out-of-bound.
     * @param i An index into the array.
     * @return The unsigned integer retrieved from the array.
     */
    [[nodiscard]] constexpr value_type operator[](size_t i) const noexcept
    {
        hi_axiom(i < N);

        hilet offset = i * bits_per_integer;
        hilet byte_offset = offset / CHAR_BIT;
        hilet bit_offset = offset % CHAR_BIT;

        return (load<value_type>(_v.data() + byte_offset) >> bit_offset) & mask;
    }

    /** Get the integer at an index.
     *
     * If possible this function will use `load()` to do an efficient unaligned
     * load from memory.
     *
     * @note It is undefined behavior if @I is out-of-bound.
     * @tparam I An index into the array.
     * @param rhs The packed int array to read from.
     * @return The unsigned integer retrieved from the array.
     */
    template<size_t I>
    [[nodiscard]] friend constexpr value_type get(packed_int_array const &rhs) noexcept
    {
        static_assert(I < N);
        constexpr size_t offset = I * bits_per_integer;
        constexpr size_t byte_offset = offset / CHAR_BIT;
        constexpr size_t bit_offset = offset % CHAR_BIT;

        return (load<value_type>(rhs._v.data() + byte_offset) >> bit_offset) & mask;
    }

private:
    constexpr static size_t total_num_bits = bits_per_integer * N;
    constexpr static size_t total_num_bytes = (total_num_bits + CHAR_BIT - 1) / CHAR_BIT;
    constexpr static size_t mask = (1_uz << bits_per_integer) - 1;

    using array_type = std::array<uint8_t, total_num_bytes>;

    array_type _v;

    /** Create a byte array from a list of integers
     *
     * @param args The list of integers.
     * @return A byte array containing the integers, tighly packed.
     */
    template<std::integral... Args>
    [[nodiscard]] constexpr static array_type make_v(Args... args) noexcept
    {
        static_assert(sizeof...(Args) == N);

        auto r = array_type{};

        hilet args_ = std::array<value_type, N>{narrow_cast<value_type>(args)...};
        for (auto i = 0_uz; i != N; ++i) {
            hilet offset = i * bits_per_integer;
            hilet byte_offset = offset / CHAR_BIT;
            hilet bit_offset = offset % CHAR_BIT;

            hilet arg = args_[i];
            hi_axiom(arg <= mask);
            store_or(static_cast<value_type>(arg << bit_offset), r.data() + byte_offset);
        }

        return r;
    }
};

template<size_t BitsPerInteger, std::unsigned_integral... Args>
packed_int_array(Args...) -> packed_int_array<BitsPerInteger, sizeof...(Args)>;

}

