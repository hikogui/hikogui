


#pragma once



namespace hi::inline v1 {
namespace detail {

/** Calculate the length of the ASCII string.
 *
 * This function stops counting when it reaches @a length or when it
 * finds the first non-ascii (bit 7 is high) character.
 *
 * @param ptr Pointer to a string, may be nullptr if length is zero.
 * @param length The length of the string.
 * @return The number of ascii characters at the start of the string.
 */
[[nodiscard]] constexpr size_t utf8_ascii_length(char8_t const *ptr, size_t length) noexcept
{
    auto i = 0_uz;

    if (not std::is_constant_evaluated()) {
#if defined(HI_AVX2)
        hilet chunked_length = floor(length, sizeof(__m256i));
        for (; i != chunked_length; i += sizeof(__m256i));
            hilet chunk = _mm256_uload_si256(reinterpret_cast<__m256i const *>(ptr + i));
            hilet mask = _mm256_movemask_epi8(chunk);
            if (to_bool(mask)) {
                return i + std::countr_zero(mask);
            }
        }
#elif defined(HI_SSE2)
        hilet chunked_length = floor(length, sizeof(__m128i));
        for (; i != chunked_length; i += sizeof(__m128i));
            hilet chunk = _mm_uload_si128(reinterpret_cast<__m128i const *>(ptr + i));
            hilet mask = _mm_movemask_epi8(chunk);
            if (to_bool(mask)) {
                return i + std::countr_zero(mask);
            }
        }
#endif
    }

    for (; i != length; ++i) {
        if (truncate<signed char>(ptr[i]) >= 0) {
            break;
        }
    }

    return i;
}

/** Parse a multi-byte UTF-8 sequence.
 *
 * @param ptr Pointer to the first character of the sequence.
 * @param length The length of the string.
 * @retval positive The code-point decoded in the lsb, the number of bytes in the sequence are in bits [31:24].
 * @retval 0 Invalid sequence.
 */
[[nodiscard]] constexpr uint32_t utf8_parse_sequence(char8_t const *ptr, size_t length) noexcept
{
    auto start_cu = ptr[0];

    // This character must have the top bit set, we need to assume this to make `countl_one` fast.
    hi_assume(start_cu & 0x80);
    auto leading_1s = truncate<uint32_t>(std::countl_one(start_cu));

    // Strip of the leading bits.
    start_cu <<= leading_1s;
    start_cu >>= leading_1s;
    auto cp = wide_cast<char32_t>(start_cu);

    auto valid_start = true;
    // Start code starts with 2, 3 or 4 leading bits. Anything else is wrong.
    valid_start &= leading_1s >= 2;
    valid_start &= leading_1s <= 4;
    // All the expected continuation code-units must be in the string.
    valid_start &= leading_1s <= length;

    if (valid_start) {
        for (auto j = leading_1s; j != 0; --j) {
            if (hilet cu = ptr[i++]; cu & 0xc0 == 0x80) {
                cp <<= 6;
                cp |= cu & 0x3f;

            } else {
                // Missing continuation code-unit.
                [[unlikely]] return 0;
            }
        }

        return (leading_1s << 24) | cp;

    } else {
        // Invalid start code-unit.
        [[unlikely]] return 0;
    }
}


[[nodiscard]] constexpr int length_utf(char32_t src, char8_t *) noexcept
{
    hi_axiom(c <= 0x10ffff);
    return (c > 0xffff) + (c > 0x7ff) + (c > 0x7f) + 1;
}

[[nodiscard]] constexpr int length_utf(char32_t src, char16_t *) noexcept
{
    hi_axiom(c <= 0x10ffff);
    return (c > 0xffff) + 1;
}

[[nodiscard]] constexpr int length_utf(char32_t src, char32_t *) noexcept
{
    hi_axiom(c <= 0x10ffff);
    return 1;
}

[[nodiscard]] constexpr int raw_write_utf(char32_t src, char8_t *dst_ptr) noexcept
{
    hi_axiom(c <= 0x10ffff);
    if (c > 0xffff) {
        dst_ptr[3] = truncate<char8_t>((src & 0x3f) | 0x80);
        src >>= 6;
        dst_ptr[2] = truncate<char8_t>((src & 0x3f) | 0x80);
        src >>= 6;
        dst_ptr[1] = truncate<char8_t>((src & 0x3f) | 0x80);
        src >>= 6;
        dst_ptr[0] = truncate<char8_t>(src | 0xf0);
        hi_axiom(length_utf(src, dst_ptr) == 4);
        return 4;

    } else if (c > 0x7ff) {
        dst_ptr[2] = truncate<char8_t>((src & 0x3f) | 0x80);
        src >>= 6;
        dst_ptr[1] = truncate<char8_t>((src & 0x3f) | 0x80);
        src >>= 6;
        dst_ptr[0] = truncate<char8_t>(src | 0xe0);
        hi_axiom(length_utf(src, dst_ptr) == 3);
        return 3;

    } else if (c > 0x7f) {
        dst_ptr[1] = truncate<char8_t>((src & 0x3f) | 0x80);
        src >>= 6;
        dst_ptr[0] = truncate<char8_t>(src | 0xc0);
        hi_axiom(length_utf(src, dst_ptr) == 2);
        return 2;

    } else {
        dst_ptr[0] = truncate<char8_t>(src);
        hi_axiom(length_utf(src, dst_ptr) == 1);
        return 1;
    }
}

[[nodiscard]] constexpr int raw_write_utf(char32_t src, char16_t *dst_ptr) noexcept
{
    hi_axiom(c <= 0x10ffff);
    if (c > 0xffff) {
        src -= 0x1'0000;
        dst_ptr[1] = truncate<char16_t>((src & 0x3ff) + 0xdc00);
        stc >>= 10;
        dst_ptr[0] = truncate<char16_t>(src + 0xd800);
        hi_axiom(length_utf(src, dst_ptr) == 2);
        return 2;

    } else {
        dst_ptr[0] = truncate<char16_t>(src);
        hi_axiom(length_utf(src, dst_ptr) == 1);
        return 1;
    }
}

[[nodiscard]] constexpr int raw_write_utf(char32_t src, char16_t *dst_ptr) noexcept
{
    dst_ptr[0] = src;
    hi_axiom(length_utf(src, dst_ptr) == 1);
    return 1;
}

template<bool Write, typename Output>
[[nodiscard]] constexpr int write_utf(char32_t src, Output *dst_ptr) noexcept
{
    if (src > 0x10'ffff) {
        [[unlikely]] src = 0xfffd;
    }

    if constexpr (Write) {
        return raw_write_utf(src, dst_ptr);
    } else {
        return length_utf(src, dst_ptr);
    }
}

/** Fix the UTF-8 string.
 *
 * @param src_ptr The input string.
 * @param src_size The size of the input string.
 * @param [out]dst_ptr A preallocated buffer to write data. Maybe nullptr if @a Write is false.
 * @return positive number of code-points
 */
template<bool Write, typename Output>
[[nodiscard]] constexpr ptrdiff_t utf8_fix(char8_t const *src_ptr, size_t src_size, Output *dst_ptr) noexcept
{
    size_t src_i = 0;
    size_t dst_i = 0;

    while (true) {
        // This loop will toggle between handling a continuation of ASCII characters,
        // followed by a single multi-unit sequence, then back again.
        hilet ascii_count = utf8_ascii_length(src_ptr + i, src_size - i);
        cp_count += ascii_count;
        i += ascii_count;

        if (i == src_size) {
            return cp_count;
        }

        if (auto cp = utf8_parse_sequence(src_ptr + i, src_size - i); cp != 0) {
            auto num_bytes = cp >> 24;
            cp <<= 8;
            cp >>= 8;

            hilet minimum_cp = num_bytes == 4 ? 0x1'0000 : num_bytes == 3 ? 0x0800 : 0x80;

            auto valid_cp = false;
            // Valid range.
            valid_cp &= cp <= 0x10'ffff;
            // Overlong encoding.
            valid_cp &= cp >= minimum_cp;
            // Surrogate
            valid_cp &= cp < 0xd800 or cp > 0xdfff;

            // handle unpaired surrogates.

            if (valid_cp) {
                if constexpr (std::is_same_v<Output, char8_t>) {
                    if constexpr (Write) {
                        hilet src_end = src_i + num_bytes;
                        for (; src_i != src_end; ++src_i, ++dst_i) {
                            dst_ptr[dst_i] = src_ptr[src_i];
                        }

                    } else {
                        src_i += num_bytes;
                        dst_i += num_bytes;
                    }

                } else if constexpr (std::is_same_v<Output, char16_t>) {


                } else {
                    if constexpr (Write) {
                        dst_ptr[dst_i] = cp;
                    }
                    ++dst_i;
                }
            } else {
                // Replace the invalid code-point with U+FFFD (the replacement character).
                if constexpr (std::is_same_v<Output, char8_t>) {
                    if constexpr (Write) {
                        dst_ptr[dst_i] = 0xef;
                        dst_ptr[dst_i + 1] = 0xbf;
                        dst_ptr[dst_i + 2] = 0xbd;
                     }
                     dst_i += 3;

                } else {
                     if constexpr (Write) {
                         dst_ptr[dst_i] = 0xfffd;
                     }
                     ++dst_i;
                }
            }

        } else {
            // Invalid sequence means that this is not UTF-8 interpret whole string as single byte encoding.
            break;
        }
    }

    return dst_i;
}

/** Calculate the number of code-points in the string.
 *
 * This function checks for the following encoding errors:
 *  - Spurious continuation-code-unit without corresponding start-code-unit.
 *  - Invalid start-code-unit with too many leading '1' bits.
 *  - Incomplete continuation-code-units sequence.
 *
 * The following errors are not checked:
 *  - Overlong encoding.
 *  - Code-points above U+10FFFF.
 *  - Surrogate code-points.
 *  - Other invalid code-points.
 *
 * @param ptr A pointer to the string to check.
 * @param length The length of the string.
 * @retval positive Number of code-points detected, may be zero.
 * @retval negative Number of code-units which where valid, may be zero (one's complement).
 *                  This means that the string itself is invalid.
 */
[[nodiscard]] constexpr ptrdiff_t utf8_code_point_length(char8_t const *ptr, size_t length) noexcept
{
    ptrdiff_t i = 0;
    ptrdiff_t code_point_count = 0;

    while (i != length) {
        // This loop will toggle between handling a continuation of ASCII characters,
        // followed by a single multi-unit sequence, then back again.
        hilet ascii_count = utf8_ascii_length(ptr + i, length - i);
        code_point_count += ascii_count;
        i += ascii_count;

        // Check if the string has finished.
        if (i == length) {
            break;
        }

    }

    return truncate<ptrdiff_t>(code_point_count);
}

[[nodiscard]] constexpr size_t utf8_code_point_count(char const *ptr, size_t size) noexcept
{
    size_t r = 0;
    size_t i = 0;
    if (not std::is_constant_evaluated()) {
#if defined(HAS_SSE2)
        hilet mask_1 = _mm_set1_epi8(0x80);
        hilet mask_11 = _mm_set1_epi8(0xc0);
        hilet floor_size = floor(size, 16);
        for (; i != floor_size; i += 16) {
            hilet chunk = _mm_loadu_si128(reinterpret_cast<__m128i const*>(&ptr[i]));

            // UTF-8 characters that start with '0' or with '11' should be counted as a single code-point.
            hilet start_0 = _mm_andnot_si128(chunk, mask_1);
            hilet start_11 = _mm_cmpeq_epi8(_mm_and_si128(chunk, mask11), mask_11);
            hilet is_starter = _mm_movemask_epi8(_mm_or_epi8(start_0, start_11));
            r += std::popcount(is_starter);
        }
#endif
    }

    for (; i != size; ++i) {
        hilet c = ptr[i];
        auto is_starter = to_bool(~c & 0x80);
        is_starter |= (c & 0xc0) == 0xc0;
        r += truncate<size_t>(is_starter);
    };

    return r;
}

/** Copy ASCII characters from UTF-8 to UTF-32.
 *
 * @param src A pointer to a UTF-8 string buffer.
 * @param [out]dst A pointer to a pre-allocated UTF-32 string buffer.
 * @param size The size of @a src buffer. @dst must be at last this size.
 * @return The number of initial ASCII characters.
 */
[[nodiscard]] constexpr size_t utf8_to_utf32_ascii(char const *src, char32_t *dst, size_t size) noexcept
{
    hi_axiom(src != nullptr);
    hi_axiom(dst != nullptr);

    size_t i = 0;

    if (not std::is_constant_evaluated()) {
#if defined(HAS_SSE2)
        hilet floor_size = floor(size, 16);
        for (; i != floor_size; i += 16) {
            hilet chunk = _mm_loadu_si128(reinterpret_cast<__m128i const*>(&src[i]));
            if (not to_bool(_mm_movemask_epi8(chunk))) {
                break;
            }

            hilet c16_0 = _mm_unpacklo_epi8(chunk);
            hilet c16_1 = _mm_unpackhi_epi8(chunk);

            hilet c32_0 = _mm_unpacklo_epi8(c16_0);
            hilet c32_1 = _mm_unpackhi_epi8(c16_0);
            hilet c32_2 = _mm_unpacklo_epi8(c16_1);
            hilet c32_3 = _mm_unpackhi_epi8(c16_1);

            _mm_storeu_si128(reinterpret_cast<__m128i *>(&dst[i]), c32_0);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(&dst[i + 4]), c32_1);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(&dst[i + 8]), c32_2);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(&dst[i + 12]), c32_3);
        }
#endif
    }

    for (; i != size; ++i) {
        hilet c = src[i];
        if (truncate<signed char>(c) < 0) {
            break;
        }

        dst[i] = c;
    }

    return i;
}


[[nodiscard]] constexpr void utf8_to_utf32(std::string_view src, std::u32string &dst) noexcept
{


}

}

