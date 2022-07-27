// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "char_encoding.hpp"
#include "../cast.hpp"
#include "../required.hpp"
#include "../architecture.hpp"
#include <cstdint>
#include <utility>

namespace hi::inline v1 {

template<>
struct char_encoder<"ascii"> {
    using char_type = char;

    [[nodiscard]] constexpr char_encoder_result read(char_type const *ptr, size_t size) const noexcept
    {
        hi_axiom(size != 0);
        
        hilet c = truncate<uint8_t>(*ptr);
        if (c < 0x80) {
            return c;
        } else {
            return {0xfffd, 1, false};
        }
    }

    template<bool Write>
    [[nodiscard]] constexpr char_encoder_result write(char32_t code_point, char_type *ptr, size_t size) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        if (code_point < 0x0100) {
            *ptr = truncate<char>(code_point);
            return {0, 1};
        } else {
            *ptr = '?';
            return {0, 1, false};
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
