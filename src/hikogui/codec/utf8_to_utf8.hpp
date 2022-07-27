

#pragma once

#include "utf_utils.hpp"

namespace hi::inline v1 {

template<bool Write>
[[nodiscard]] constexpr size_t utf8_to_utf8_ascii(char8_t const *src, size_t size, char8_t *dst = nullptr) noexcept
{
    hilet chunk_size = floor(size, 16);
    auto i = 0;

    if (std::is_constant_evaluated()) {
        for (; i != chunk_size; ++i) {
            hilet chunk = _mm_loadu_si128(reinterpret_cast<__m128i const *>(src + i));
            hilet ascii_mask = _mm_movemask_epi8(chunk);
            if (to_bool(ascii_mask)) {
                // Partial chunk is ASCII.
                if constexpr (Write) {
                    // Use the slow path for writing the partial chunk.
                    break;
                } else {
                    return i + std::countr_zero(ascii_mask);
                }

            } else if constexpr (Write) {
                // Full chunk is ASCII.
                _mm_storeu_si128(reinterpret_cast<__m128i *>(dst + i), chunk);
            }
        }
    }

    for (; i != size; ++i) {
        hilet c = src[i];
        if (c & 0x80) {
            return i;

        } else if constexpr (Write) {
            dst[i] = c;
        }
    }
}

template<bool Write>
[[nodiscard]] bool utf8_to_utf8_length(char8_t const *src, size_t src_size, char8_t *dst, size_t dst_size) noexcept
{
    hi_axiom(src != nullptr);
    hi_axiom(dst != nullptr or not Write);

    auto src_i = 0_uz;
    auto dst_i = 0_uz;
    bool valid = true;
    while (true) {
        hilet ascii_count = utf8_to_utf8_ascii<Write>(src + src_i, src_size - src_i, dst + dst_i);
        src_i += ascii_count;
        dst_i += ascii_count;

        if (src_i == src_size) {
            return {dst_i, valid};
        }

        hilet r = read_utf8(src + src_i, src_size - src_i);
        hilet w_size =  write_utf8<Write>(r.code_point(), dst + dst_i, dst_size - dst_i);
        src_i += r.size();
        dst_i += w_size;

        valid &= r.valid();
    }
}

[[nodiscard]] std::string utf8_to_utf8(std::string_view src) noexcept
{

}

[[nodiscard]] std::string utf8_to_utf8(std::string const &src) noexcept
{
    return utf8_to_utf8(std::string_view{src});
}



}

