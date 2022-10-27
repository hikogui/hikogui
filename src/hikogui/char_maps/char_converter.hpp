// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file char_maps/char_converter.hpp Definition of the char_converter<From,To> functor.
 * @ingroup char_maps
 */

#pragma once

#include "../fixed_string.hpp"
#include "../memory.hpp"
#include "../endian.hpp"
#include <string>
#include <string_view>
#if defined(HI_HAS_SSE2)
#include <emmintrin.h>
#endif

namespace hi { inline namespace v1 {

/** Character encoder/decoder template.
 * @ingroup char_maps
 *
 * @tparam Encoding a string-tag representing the encoding.
 *
 * Implementations have to define the following members:
 *
 * ### Character type.
 * ```cpp
 * char_type
 * ```
 *
 *
 * ### Read a single code-point
 * ```cpp
 * constexpr std::pair<char32_t, bool> read(char_type const *& ptr, char_type const *last) const noexcept
 * ```
 *  - _[in,out]ptr_ Pointer to the first code-unit of the code-point to read.
 *    on return the pointer will point beyond the last code-unit of the code-point that was read.
 *  - _last_ A pointer pointing one beyond the string.
 *  - _return (code-point, valid)_ The function will always return a code-point,
 *    even if there was a parse error, in this case `valid` is false.
 *
 *
 * ### Determine number of code-units for a code-point.
 * ```cpp
 * constexpr std::pair<uint8_t, bool> size(char32_t code_point) const noexcept
 * ```
 *  - _code_point_ The code-point to encode.
 *  - _return (count, valid)_ If the code-point can not be encoded `valid` will be false,
 *    end count will contain the number of code-unit needed to
 *    encode a replacement character.
 *
 *
 * ### Encode a single code-point.
 * ```cpp
 * constexpr void write(char32_t code_point, char_type *&ptr) const noexcept
 * ```
 *  - _code_point_ The code-point to encode.
 *  - _[in,out]ptr_ The pointer where the code-units will be written. On return
 *    will contain the pointer beyond where the code-units where written.
 *    It is undefined behavior if the ptr does not point to a valid buffer
 *    where all the code-units can be written to.
 *
 *
 * ### Read a chunk of ASCII characters.
 * ```cpp
 * __m128i read_ascii_chunk16(char_type const *ptr) const noexcept
 * ```
 * `read_ascii_chunk16()` returns a 16 byte register. The implementation of this function must set the high-bit of
 * each non-ASCII character.
 *  - `ptr` A pointer to the first character of a chunk of 16 characters.
 *  - _return_ 16 bytes in a register, bit 7 must be '1' if the character is not ASCII.
 *
 *
 * ### Write a chunk of ASCII characters.
 * ```cpp
 * void write_ascii_chunk16(__m128i chunk, char_type *ptr) const noexcept
 * ```
 *  - _chunk_ A chunk of 16 ascii characters. bit 7 is always '0'.
 *  - _ptr_ The pointer to the first code-unit where the ASCII characters must be written to.
 */
template<fixed_string Encoding>
struct char_map;

/** A converter between character encodings.
 *
 * @ingroup char_maps
 * @tparam From a string-tag matching an existing `char_map<From>`
 * @tparam To a string-tag matching an existing `char_map<To>`
 */
template<fixed_string From, fixed_string To>
struct char_converter {
public:
    using from_encoder_type = char_map<From>;
    using to_encoder_type = char_map<To>;
    using from_char_type = from_encoder_type::char_type;
    using to_char_type = to_encoder_type::char_type;
    using from_string_type = std::basic_string<from_char_type>;
    using to_string_type = std::basic_string<to_char_type>;

    /** Convert text between the given encodings.
     *
     * @tparam OutRange The output type
     * @param src The text to be converted.
     * @return The converted text.
     */
    template<typename OutRange, typename InRange>
    [[nodiscard]] constexpr OutRange convert(InRange&& src) const noexcept
    {
        using std::cbegin;
        using std::cend;
        using std::begin;
        using std::end;

        hilet[size, valid] = _size(cbegin(src), cend(src));

        auto r = OutRange{};
        if constexpr (From == To and std::is_same_v<InRange, OutRange>) {
            if (valid) {
                r = std::forward<InRange>(src);
                // If and identity conversion is requested and the src is valid, then shortcut by return the src.
                return r;
            }
        }

        if (size == 0) {
            return r;
        }

        r.resize(size);
        if (From == To and valid) {
            hi_axiom(size != 0);

            using std::size;
            std::memcpy(std::addressof(*begin(r)), std::addressof(*cbegin(src)), size(src) * sizeof(from_char_type));
        } else {
            _convert(cbegin(src), cend(src), begin(r));
        }
        return r;
    }

    /** Convert text between the given encodings.
     *
     * @tparam OutRange The output type
     * @param first An iterator pointing to the first character to be converted.
     * @param last An iterator pointing one beyond the last character to be converted, or a sentinel.
     * @return The converted text.
     */
    template<typename OutRange, typename It, typename EndIt>
    [[nodiscard]] constexpr OutRange convert(It first, EndIt last) const noexcept
    {
        using std::begin;

        hilet[size, valid] = _size(first, last);
        auto r = OutRange{};
        if (size == 0) {
            return r;
        }

        r.resize(size);
        if (From == To and valid) {
            hi_axiom(size != 0);

            std::memcpy(std::addressof(*begin(r)), std::addressof(*first), std::distance(first, last) * sizeof(from_char_type));
        } else {
            _convert(first, last, begin(r));
        }
        return r;
    }

    /** Read text from a byte array.
     *
     * @tparam OutRange The output type
     * @param ptr A pointer to a byte array containing the text in the `From` encoding.
     * @param size The number of bytes in the array.
     * @param endian The endianness of characters in the array, used as a hint.
     * @return The converted text.
     */
    template<typename OutRange = std::basic_string<to_char_type>>
    [[nodiscard]] OutRange read(void const *ptr, size_t size, std::endian endian = std::endian::native) noexcept
    {
        hi_assert_not_null(ptr);

        hilet num_chars = size / sizeof(from_char_type);

        endian = from_encoder_type{}.guess_endian(ptr, size, endian);
        if (endian == std::endian::native) {
            if (floor(ptr, sizeof(from_char_type)) == ptr) {
                return convert<OutRange>(
                    reinterpret_cast<from_char_type const *>(ptr), reinterpret_cast<from_char_type const *>(ptr) + num_chars);
            } else {
                auto tmp = std::basic_string<from_char_type>{};
                tmp.resize(num_chars);
                std::memcpy(std::addressof(*tmp.begin()), ptr, num_chars * sizeof(from_char_type));
                return convert<OutRange>(std::move(tmp));
            }
        } else {
            auto tmp = std::basic_string<from_char_type>{};
            tmp.resize(num_chars);
            std::memcpy(std::addressof(*tmp.begin()), ptr, num_chars * sizeof(from_char_type));
            for (auto& c : tmp) {
                c = byte_swap(c);
            }
            return convert<OutRange>(std::move(tmp));
        }
    }

    /** Convert text between the given encodings.
     *
     * @param src The text to be converted.
     * @return The converted text as a std::basic_string<to_char_type>.
     */
    template<typename InRange>
    [[nodiscard]] constexpr to_string_type operator()(InRange&& src) const noexcept
    {
        return convert<to_string_type>(std::forward<InRange>(src));
    }

private:
#if defined(HI_HAS_SSE2)
    using chunk16_type = __m128i;
#else
    using chunk16_type = void;
#endif

    constexpr static bool _has_read_ascii_chunk16 = true;
    constexpr static bool _has_write_ascii_chunk16 = true;

    template<typename It, typename EndIt>
    [[nodiscard]] constexpr void _size_ascii(It& it, EndIt last, size_t& count) const noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE2)
            if constexpr (_has_read_ascii_chunk16 and _has_write_ascii_chunk16) {
                while (std::distance(it, last) >= 16) {
                    hilet chunk = from_encoder_type{}.read_ascii_chunk16(it);
                    hilet ascii_mask = _mm_movemask_epi8(chunk);
                    if (ascii_mask) {
                        // This chunk contains non-ASCII characters.
                        auto partial_count = std::countr_zero(truncate<uint16_t>(ascii_mask));
                        it += partial_count;
                        count += partial_count;
                        break;
                    }
                    it += 16;
                    count += 16;
                }
            }
#endif
        }
    }

    template<typename SrcIt, typename SrcEndIt, typename DstIt>
    void _convert_ascii(SrcIt& src, SrcEndIt src_last, DstIt& dst) const noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE2)
            if constexpr (_has_read_ascii_chunk16 and _has_write_ascii_chunk16) {
                while (std::distance(src, src_last) >= 16) {
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

    template<typename It, typename EndIt>
    [[nodiscard]] constexpr std::pair<size_t, bool> _size(It it, EndIt last) const noexcept
    {
        auto count = 0_uz;
        auto valid = true;
        while (true) {
            // This loop toggles between converting chunks of ASCII characters and converting
            // a single non-ASCII character.
            _size_ascii(it, last, count);

            if (it == last) {
                break;
            }

            hilet[code_point, read_valid] = from_encoder_type{}.read(it, last);
            valid &= read_valid;

            hilet[write_count, write_valid] = to_encoder_type{}.size(code_point);
            count += write_count;
            valid &= write_valid;
        }

        return {count, valid};
    }

    template<typename SrcIt, typename SrcEndIt, typename DstIt>
    void _convert(SrcIt src, SrcEndIt src_last, DstIt dst) const noexcept
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

}} // namespace hi::v1