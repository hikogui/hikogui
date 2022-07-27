// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../fixed_string.hpp"
#include <string>
#include <string_view>

namespace hi::inline v1 {

/** Character encoder/decoder template.
 *
 * Implementations have to define these optional methods:
 *  - `[[nodiscard]] constexpr char_encoder_result read(char_type const *ptr, size_t size) const noexcept`
 *  - `template<bool Write> [[nodiscard]] constexpr char_encoder_result write(char32_t code_point, char_type *ptr, size_t size)
 *     const noexcept`
 *  - `[[nodiscard]] __m128i read_ascii_chunk16(char_type const *ptr) noexcept`
 *  - `void write_ascii_chunk16(__m128i chunk, char_type *ptr) const noexcept`
 *
 * Implementations are required to add the following tyoes:
 *  - char_type
 *
 * `read_ascii_chunk16()` returns a 16 byte register. The implementation of this function must set the high-bit of
 * each non-ASCII character.
 */
template<fixed_string Encoding>
struct char_encoder {
};

class char_encoder_result {
public:
    constexpr ~char_encoder_result() = default;
    constexpr char_encoder_result() noexcept = default;
    constexpr char_encoder_result(char_encoder_result const&) noexcept = default;
    constexpr char_encoder_result(char_encoder_result&&) noexcept = default;
    constexpr char_encoder_result& operator=(char_encoder_result const&) noexcept = default;
    constexpr char_encoder_result& operator=(char_encoder_result&&) noexcept = default;

    constexpr char_encoder_result(char32_t code_point, size_t size = 1, bool value = true) noexcept :
        _code_point(truncate<uint32_t>(code_point)), _size(trucate<uint32_t>(size)), _valid(truncate<uint32_t>(valid))
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(size < 0x80);
    }

    [[nodiscard]] constexpr char32_t code_point() const noexcept
    {
        return truncate<char32_t>(_code_point);
    }

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return wide_cast<size_t>(_size);
    }

    [[nodiscard]] constexpr bool valid() const noexcept
    {
        return to_bool(_valid);
    }

    [[nodiscard]] char_encoder_result make_invalid() const noexcept
    {
        auto r = *this;
        r._valid = 0;
        return r;
    }

private:
    uint32_t _code_point : 24 = 0;
    uint32_t _size : 7 = 0;
    uint32_t _valid : 1 = 0;

    [[nodiscard]] constexpr friend bool operator==(char_encoder_result const&, char_encoder_result const&) noexcept = default;
    [[nodiscard]] constexpr friend auto operator<=>(char_encoder_result const&, char_encoder_result const&) noexcept = default;
};

template<fixed_string From, fixed_string To>
class char_converter {
public:
    using from_encoder_type = char_encoder<From>;
    using to_encoder_type = char_encoder<To>;
    using from_char_type = from_encoder_type::char_type;
    using to_char_type = to_encoder_type::char_type;

    [[nodiscard]] constexpr std::basic_string<to_char_type> convert(std::basic_string_view<from_char_type> str) noexcept
    {
        hilet[size, valid] = _convert<false>(str.data(), str.size(), nullptr, 0);

        auto r = std::basic_string<to_char_type>{size, to_char_type{}};
        _convert<true>(str.data(), str.size(), r.data(), r.size());
        return r;
    }

    [[nodiscard]] constexpr std::basic_string<to_char_type> convert(std::basic_string<from_char_type>&& str) noexcept
    {
        hilet[size, valid] = _convert<false>(str.data(), str.size(), nullptr, 0);

        if (From == To and valid) {
            // Short-cut if the input string is valid.
            return str;
        }

        auto r = std::basic_string<to_char_type>{size, to_char_type{}};
        _convert<true>(str.data(), str.size(), r.data(), r.size());
        return r;
    }

private:
#if defined(HI_HAS_SSE2)
    using chunk16_type = __m128i;
#else
    using chunk16_type = void;
#endif

    from_encoder_type _from = from_encoder_type{};
    to_encoder_type _to = to_encoder_type{};

    constexpr static _has_read_ascii_chunk16 = requires(from_char_type const *src)
    {
        from_encoder_type{}.read_ascii_chunk16(src);
    };

    constexpr static _to_write_ascii_chunk16 = requires(to_char_type * dst)
    {
        to_encoder_type{}.write_ascii_chunk16<true>(chunk16_type{}, dst);
    };

    template<bool Write>
    [[nodiscard]] constexpr size_t _convert_ascii(from_char_type const *src, size_t src_size, to_char_type *dst) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE2)
            if constexpr (_has_read_ascii_chunk16 and _has_write_ascii_chunk16) {
                hilet chunked_size = floor(src_size, 16);
                auto i = 0_uz;
                for (; i != chunked_size; i += 16) {
                    hilet chunk = _from.read_ascii_chunk(src + i);
                    hilet ascii_mask = _mm_movemask_epi8(chunk);
                    if (ascii_mask) {
                        // The chunk contains non-ASCII characters.
                        if constexpr (Write) {
                            // When writing we handle only full chunks, handle a partial chunk on the slow-path.
                            return i;
                        } else {
                            // For a length calculation we can do it more precise.
                            return i + std::countr_zero(ascii_mask);
                        }
                    } else {
                        // The complete chunk only contains ASCII characters.
                        if constexpr (Write) {
                            _to.write_ascii_chunk(chunk, dst + i);
                        }
                    }
                }
                // At the end of the string, ignore the partial chunk on the end and handle that in the slow-path.
                return i;
            }
#endif
        }
        // Handle ASCII on the slow-path.
        return 0;
    }

    template<bool Write>
    constexpr std::pair<size_t, bool>
    _convert(from_char_type const *src, size_t src_size, to_char_type *dst, size_t dst_size) noexcept
    {
        auto src_i = 0_uz;
        auto dst_i = 0_uz;
        auto valid = true;
        while (true) {
            // This loop toggles between converting chunks of ASCII characters and converting
            // a single non-ASCII character.
            hilet ascii_count = _convert_ascii<Write>(src + src_i, src_size - src_i, dst + dst_i);
            src_i += ascii_count;
            dst_i += ascii_count;

            if (src_i == src_size) {
                break;
            }

            hilet from_r = _from::read(src + src_i, src_size - src_i);
            src_i += from_r.size();
            valid &= from_r.valid();

            hilet to_r = _to::write<Write>(from_r.code_point(), dst + dst_i, dst_size - dst_i);
            dst_i += to_r.size();
            valid &= to_r.valid();
        }

        return {dst_i, valid};
    }
};

} // namespace hi::inline v1
