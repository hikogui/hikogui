// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <span>
#include <cstddef>

export module hikogui_font_otype_loca;
import hikogui_font_otype_utilities;
import hikogui_utility;

export namespace hi { inline namespace v1 {

[[nodiscard]] std::pair<size_t, size_t> otype_loca16_get(std::span<std::byte const> bytes, hi::glyph_id glyph_id)
{
    hilet entries = implicit_cast<big_uint16_buf_t>(bytes, bytes.size() / sizeof(big_uint16_buf_t));
    hilet first = wide_cast<size_t>(*hi_check_at(entries, *glyph_id)) * 2;
    hilet last = wide_cast<size_t>(*hi_check_at(entries, *glyph_id + 1)) * 2;
    hi_check(first <= last, "'loca' table has invalid entries.");
    return {first, last};
}

[[nodiscard]] std::pair<size_t, size_t> otype_loca32_get(std::span<std::byte const> bytes, hi::glyph_id glyph_id)
{
    hilet entries = implicit_cast<big_uint32_buf_t>(bytes, bytes.size() / sizeof(big_uint32_buf_t));
    hilet first = wide_cast<size_t>(*hi_check_at(entries, *glyph_id));
    hilet last = wide_cast<size_t>(*hi_check_at(entries, *glyph_id + 1));
    hi_check(first <= last, "'loca' table has invalid entries.");
    return {first, last};
}

[[nodiscard]] std::pair<size_t, size_t>
otype_loca_get(std::span<std::byte const> loca_bytes, hi::glyph_id glyph_id, bool loca_is_offset32)
{
    if (loca_is_offset32) {
        return otype_loca32_get(loca_bytes, glyph_id);
    } else {
        return otype_loca16_get(loca_bytes, glyph_id);
    }
}

[[nodiscard]] std::span<std::byte const> otype_loca_get(
    std::span<std::byte const> loca_bytes,
    std::span<std::byte const> glyf_bytes,
    hi::glyph_id glyph_id,
    bool loca_is_offset32)
{
    hilet[first, last] = otype_loca_get(loca_bytes, glyph_id, loca_is_offset32);
    hilet size = last - first;
    return hi_check_subspan(glyf_bytes, first, size);
}

}} // namespace hi::v1
