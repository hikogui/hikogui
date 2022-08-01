// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "char_converter.hpp"
#include "../cast.hpp"
#include "../required.hpp"
#include "../architecture.hpp"
#include <cstdint>
#include <utility>

namespace hi::inline v1 {

template<>
struct char_map<"iso-8859-1"> {
    using char_type = char;

    [[nodiscard]] constexpr std::pair<char32_t, bool> read(char_type const *& ptr, char_type const *last) const noexcept
    {
        hi_axiom(ptr != last);
        return {char_cast<char32_t>(*ptr++), true};
    }

    [[nodiscard]] constexpr std::pair<uint8_t, bool> size(char32_t code_point) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        if (code_point < 0x0100) {
            return {uint8_t{1}, true};

        } else {
            return {uint8_t{1}, false};
        }
    }

    constexpr void write(char32_t code_point, char_type *& ptr) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        if (code_point < 0x0100) {
            *ptr++ = char_cast<char>(code_point);
        } else {
            *ptr++ = '?';
        }
    }

#if defined(HI_HAS_SSE2)
    hi_force_inline __m128i read_ascii_chunk16(char_type const *ptr) const noexcept
    {
        return _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr));
    }

    hi_force_inline void write_ascii_chunk16(__m128i chunk, char_type *ptr) const noexcept
    {
        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr), chunk);
    }
#endif
};

} // namespace hi::inline v1
