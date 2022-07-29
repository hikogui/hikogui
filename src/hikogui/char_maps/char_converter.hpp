// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../fixed_string.hpp"
#include "../memory.hpp"
#include <string>
#include <string_view>
#if defined(HI_HAS_SSE2)
#include <emmintrin.h>
#endif

namespace hi::inline v1 {

/** Character encoder/decoder template.
 *
 * Implementations have to define these optional methods:
 *  - `read()`
 *  - `size()`
 *  - `write()`
 *  - `read_ascii_chunk16()`
 *  - `write_ascii_chunk16()`
 *
 * Implementations are required to add the following types:
 *  - char_type
 *
 * `read_ascii_chunk16()` returns a 16 byte register. The implementation of this function must set the high-bit of
 * each non-ASCII character.
 */
template<basic_fixed_string Encoding>
struct char_map {
    /** @fn constexpr std::pair<char32_t, bool> read(char_type const *& ptr, char_type const *last) const noexcept
     * @brief Read a single code-point.
     * @param [in,out]ptr Pointer to the first code-unit of the code-point to read.
     *        on return the pointer will point beyond the last code-unit of the code-point that was read.
     * @param last A pointer pointing one beyond the string.
     * @return (code-point, valid) The function will always return a code-point,
     *         even if there was a parse error, in this case `valid` is false.
     */

    /** @fn constexpr std::pair<uint8_t, bool> size(char32_t code_point) const noexcept
     * @brief Get how many code-unit are required to encode the code-point
     * @param code_point The code-point to encode.
     * @return (count, valid) If the code-point can not be encoded `valid` will be false,
     *                        end count will contain the number of code-unit needed to
     *                        encode a replacement character.
     */

    /** @fn constexpr void write(char32_t code_point, char_type *&ptr) const noexcept
     * @brief Encode a single code-unit.
     * @param code_point The code-point to encode.
     * @param [in,out]ptr The pointer where the code-units will be written. On return
     *                    will contain the pointer beyond where the code-units where written.
     *                    It is undefined behavior if the ptr does not point to a valid buffer
     *                    where all the code-units can be written to.
     */

    /** @fn __m128i read_ascii_chunk16(char_type const *ptr) const noexcept
     * @brief Read a chunk of ascii characters.
     * @param ptr A pointer to the first character of a chunk of 16 characters.
     * @return 16 bytes in a register, bit 7 must be '1' if the character is not ASCII.
     */

    /** @fn void write_ascii_chunk16(__m128i chunk, char_type *ptr) const noexcept
     * @brief Write a chunk of ascii characters.
     * @param chunk A chunk of 16 ascii characters. bit 7 is always '0'.
     * @param ptr The pointer to the first code-unit where the ASCII characters must be written to.
     */
};

template<basic_fixed_string From, basic_fixed_string To>
struct char_converter {
public:
    using from_encoder_type = char_map<From>;
    using to_encoder_type = char_map<To>;
    using from_char_type = from_encoder_type::char_type;
    using to_char_type = to_encoder_type::char_type;

    [[nodiscard]] constexpr std::basic_string<to_char_type> convert(std::basic_string_view<from_char_type> str) noexcept
    {
        hilet[size, valid] = _size(str.data(), str.data() + str.size());

        auto r = std::basic_string<to_char_type>(size, to_char_type{});
        _convert(str.data(), str.data() + str.size(), r.data());
        return r;
    }

    [[nodiscard]] constexpr std::basic_string<to_char_type> convert(std::basic_string<from_char_type>&& str) noexcept
    {
        hilet[size, valid] = _size(str.data(), str.data() + str.size());

        if (From == To and valid) {
            // Short-cut if the input string is valid.
            return str;
        }

        auto r = std::basic_string<to_char_type>(size, to_char_type{});
        _convert(str.data(), str.data() + str.size(), r.data());
        return r;
    }

private:
#if defined(HI_HAS_SSE2)
    using chunk16_type = __m128i;
#else
    using chunk16_type = void;
#endif

    constexpr static bool _has_read_ascii_chunk16 = requires(from_char_type const *src)
    {
        from_encoder_type{}.read_ascii_chunk16(src);
    };

    constexpr static bool _has_write_ascii_chunk16 = requires(to_char_type * dst)
    {
        to_encoder_type{}.write_ascii_chunk16(chunk16_type{}, dst);
    };

    [[nodiscard]] constexpr void _size_ascii(from_char_type const *& ptr, from_char_type const *last, size_t &count) const noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE2)
            if constexpr (_has_read_ascii_chunk16 and _has_write_ascii_chunk16) {
                while (ptr + 16 < last) {
                    hilet chunk = from_encoder_type{}.read_ascii_chunk16(ptr);
                    hilet ascii_mask = _mm_movemask_epi8(chunk);
                    if (ascii_mask) {
                        // This chunk contains non-ASCII characters.
                        auto partial_count = std::countr_zero(truncate<uint16_t>(ascii_mask));
                        ptr += partial_count;
                        count += partial_count;
                        break;
                    }
                    ptr += 16;
                    count += 16;
                }
            }
#endif
        }
    }

    void _convert_ascii(from_char_type const *& src, from_char_type const *src_last, to_char_type *&dst) const noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE2)
            if constexpr (_has_read_ascii_chunk16 and _has_write_ascii_chunk16) {
                while (src + 16 < src_last) {
                    hilet chunk = from_encoder_type{}.read_ascii_chunk16(src);
                    hilet ascii_mask = _mm_movemask_epi8(chunk);
                    if (ascii_mask) {
                        // This chunk contains non-ASCII characters.
                        break;
                    }
                    // The complete chunk only contains ASCII characters.
                    to_encoder_type{}.write_ascii_chunk16(chunk, dst);
                    src += 16;
                    dst += 16;
                }
            }
#endif
        }
    }

    [[nodiscard]] constexpr std::pair<size_t, bool> _size(from_char_type const *ptr, from_char_type const *last) const noexcept
    {
        auto count = 0_uz;
        auto valid = true;
        while (true) {
            // This loop toggles between converting chunks of ASCII characters and converting
            // a single non-ASCII character.
            _size_ascii(ptr, last, count);

            if (ptr == last) {
                break;
            }

            hilet[code_point, read_valid] = from_encoder_type{}.read(ptr, last);
            valid &= read_valid;

            hilet[write_count, write_valid] = to_encoder_type{}.size(code_point);
            count += write_count;
            valid &= write_valid;
        }

        return {count, valid};
    }

    void _convert(from_char_type const *src, from_char_type const *src_last, to_char_type *dst) const noexcept
    {
        while (true) {
            // This loop toggles between converting chunks of ASCII characters and converting
            // a single non-ASCII character.
            _convert_ascii(src, src_last, dst);

            if (src == src_last) {
                break;
            }

            hilet[code_point, from_valid] = from_encoder_type{}.read(src, src_last);
            to_encoder_type{}.write(code_point, dst);
        }
    }
};

} // namespace hi::inline v1
