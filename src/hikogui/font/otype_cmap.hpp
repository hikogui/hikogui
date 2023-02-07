// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "otype_utilities.hpp"
#include "font_char_map.hpp"

namespace hi { inline namespace v1 {

[[nodiscard]] inline std::span<std::byte const>
otype_cmap_find(std::span<std::byte const> bytes, uint16_t platform_id, uint16_t platform_specific_id)
{
    struct header_type {
        big_uint16_buf_t version;
        big_uint16_buf_t num_tables;
    };

    struct entry_type {
        big_uint16_buf_t platform_id;
        big_uint16_buf_t platform_specific_id;
        big_uint32_buf_t offset;
    };

    std::size_t offset = 0;

    hilet& header = implicit_cast<header_type>(offset, bytes);
    hi_check(*header.version == 0, "CMAP version is not 0");

    hilet entries = implicit_cast<entry_type>(offset, bytes, *header.num_tables);

    auto key = (truncate<uint32_t>(platform_id) << 16) | truncate<uint32_t>(platform_specific_id);
    if (hilet entry = fast_binary_search_eq<std::endian::big>(entries, key)) {
        return hi_check_subspan(bytes, *entry->offset);
    } else {
        return {};
    }
}

[[nodiscard]] inline font_char_map otype_cmap_parse_map_4(std::span<std::byte const> over_sized_bytes)
{
    struct header_type {
        big_uint16_buf_t format;
        big_uint16_buf_t length;
        big_uint16_buf_t language;
        big_uint16_buf_t seg_count_x2;
        big_uint16_buf_t search_range;
        big_uint16_buf_t entry_selector;
        big_uint16_buf_t range_shift;
    };

    auto offset = 0_uz;
    hilet& header = implicit_cast<header_type>(offset, over_sized_bytes);
    hi_axiom(*header.format == 4);
    hilet length = *header.length;
    hilet bytes = hi_check_subspan(over_sized_bytes, 0, length);

    hilet seg_count = *header.seg_count_x2 / 2;

    hilet end_codes = implicit_cast<big_uint16_buf_t>(offset, bytes, seg_count);
    offset += sizeof(uint16_t); // reservedPad
    hilet start_codes = implicit_cast<big_uint16_buf_t>(offset, bytes, seg_count);

    hilet id_deltas = implicit_cast<big_uint16_buf_t>(offset, bytes, seg_count);

    hilet id_range_offsets = implicit_cast<big_uint16_buf_t>(offset, bytes, seg_count);

    hilet glyph_id_array_count = (bytes.size() - offset) / sizeof(big_uint16_buf_t);
    hilet glyph_id_array = implicit_cast<big_uint16_buf_t>(offset, bytes, glyph_id_array_count);

    auto r = font_char_map{};
    r.reserve(seg_count);
    auto prev_end_code_point = char32_t{0};
    for (auto i = 0_uz; i != seg_count; ++i) {
        hilet end_code_point = char_cast<char32_t>(*end_codes[i]);
        hilet start_code_point = char_cast<char32_t>(*start_codes[i]);

        hi_check(start_code_point <= end_code_point, "'cmap' subtable 4, start code-point must come before end code-point.");
        hi_check(
            i == 0 or prev_end_code_point < start_code_point,
            "'cmap' subtable 4, all entries must be non-overlapping and ordered.");

        if (start_code_point == 0xffff and end_code_point == 0xffff) {
            // The last entry is a single character explicit terminator and does not need to be added.
            break;
        }

        auto id_range_offset = wide_cast<size_t>(*id_range_offsets[i]);
        if (id_range_offset == 0) {
            // Simple modulo 65536 delta on the start_code_point to get a glyph_id.
            auto start_glyph_id = *id_deltas[i];
            start_glyph_id += char_cast<uint16_t>(start_code_point);

            hi_check(
                start_glyph_id + (end_code_point - start_code_point) + 1_uz < 0xffff,
                "'cmap' subtable 4, glyph_id must be in range 0 to 0xfffe.");
            r.add(start_code_point, end_code_point, start_glyph_id);

        } else {
            // The formula from the specification:
            // `glyphIndex = *( &idRangeOffset[i] + idRangeOffset[i] / 2 + (c - startCode[i]) )`

            // Get the offset to the glyph_id_array.
            id_range_offset /= sizeof(big_uint16_buf_t);
            // Get the index in the glyph_index_table. By subtracting the rest of the id_range_offsets table.
            id_range_offset -= seg_count - i;

            // When using the glyph_index_table, add glyphs one by one.
            hilet code_point_count = end_code_point - start_code_point + 1;
            for (auto j = 0_uz; j != code_point_count; ++j) {
                hilet code_point = char_cast<char32_t>(start_code_point + j);

                hilet glyph_id = *hi_check_at(glyph_id_array, id_range_offset + j);
                hi_check(glyph_id < 0xfffe, "'cmap' subtable 4, glyph_id must be in range 0 to 0xfffe.");
                r.add(code_point, code_point, glyph_id);
            }
        }

        prev_end_code_point = end_code_point;
    }

    r.prepare();
    return r;
}

[[nodiscard]] inline font_char_map otype_cmap_parse_map_6(std::span<std::byte const> over_sized_bytes)
{
    struct header_type {
        big_uint16_buf_t format;
        big_uint16_buf_t length;
        big_uint16_buf_t language;
        big_uint16_buf_t first_code;
        big_uint16_buf_t entry_count;
    };

    auto offset = 0_uz;
    hilet& header = implicit_cast<header_type>(offset, over_sized_bytes);
    hi_axiom(*header.format == 6);
    hilet bytes = hi_check_subspan(over_sized_bytes, 0, *header.length);

    hilet entry_count = *header.entry_count;
    hilet entries = implicit_cast<big_uint16_buf_t>(offset, bytes, entry_count);

    auto r = font_char_map{};
    r.reserve(entry_count);
    auto code_point = char_cast<char32_t>(*header.first_code);
    for (auto i = 0_uz; i != entry_count; ++i, ++code_point) {
        hilet glyph_id = *entries[i];
        hi_check(glyph_id < 0xfffe, "'cmap' subtable 6, glyph_id must be in range 0 to 0xfffe.");
        r.add(code_point, code_point, glyph_id);
    }

    // Add terminator to the table.
    r.prepare();
    return r;
}

[[nodiscard]] inline font_char_map otype_cmap_parse_map_12(std::span<std::byte const> over_sized_bytes)
{
    struct header_type {
        big_uint16_buf_t format;
        big_uint16_buf_t reserved;
        big_uint32_buf_t length;
        big_uint32_buf_t language;
        big_uint32_buf_t num_groups;
    };

    struct entry_type {
        big_uint32_buf_t start_char_code;
        big_uint32_buf_t end_char_code;
        big_uint32_buf_t start_glyph_id;
    };

    auto offset = 0_uz;
    hilet& header = implicit_cast<header_type>(offset, over_sized_bytes);
    hi_axiom(*header.format == 12);
    hilet bytes = hi_check_subspan(over_sized_bytes, 0, *header.length);

    hilet entries = implicit_cast<entry_type>(offset, bytes, *header.num_groups);

    auto r = font_char_map{};
    r.reserve(*header.num_groups);
    for (hilet& entry : entries) {
        hilet start_code_point = char_cast<char32_t>(*entry.start_char_code);
        hilet end_code_point = char_cast<char32_t>(*entry.end_char_code);
        hi_check(start_code_point <= end_code_point, "'cmap' subtable 12, has invalid code-point range.");

        hilet start_glyph_id = *entry.start_glyph_id;
        hi_check(
            start_glyph_id + (end_code_point - start_code_point) + 1_uz < 0xffff,
            "'cmap' subtable 12, glyph_id must be in range 0 to 0xfffe.");
        r.add(start_code_point, end_code_point, narrow_cast<uint16_t>(start_glyph_id));
    }

    // Add terminator to the table.
    r.prepare();
    return r;
}

[[nodiscard]] inline font_char_map otype_cmap_parse_map(std::span<std::byte const> bytes)
{
    // The first 16 bits of a cmap sub-table always contain the format.
    hilet format = *implicit_cast<big_uint16_buf_t>(bytes);

    switch (format) {
    case 4:
        return otype_cmap_parse_map_4(bytes);
    case 6:
        return otype_cmap_parse_map_6(bytes);
    case 12:
        return otype_cmap_parse_map_12(bytes);
    default:
        // Unknown format, let otype_cmap_parse try the next sub-table.
        return {};
    }
}

[[nodiscard]] inline font_char_map otype_cmap_parse(std::span<std::byte const> bytes)
{
    constexpr auto search_order = std::array{
        std::pair{uint16_t{0}, uint16_t{4}}, // Unicode - Unicode 2.0 non-BMP
        std::pair{uint16_t{0}, uint16_t{3}}, // Unicode - Unicode 2.0 BMP-only
        std::pair{uint16_t{0}, uint16_t{2}}, // Unicode - ISO 10646 1993
        std::pair{uint16_t{0}, uint16_t{1}}, // Unicode - Version 1.1
        std::pair{uint16_t{3}, uint16_t{10}}, // Microsoft Windows - Unicode 32-bit
        std::pair{uint16_t{3}, uint16_t{1}}, // Microsoft Windows - Unicode 16-bit
        std::pair{uint16_t{3}, uint16_t{0}} // Microsoft Windows - Symbol.
    };

    for (hilet[platform_id, platform_specific_id] : search_order) {
        if (hilet map_bytes = otype_cmap_find(bytes, platform_id, platform_specific_id); not map_bytes.empty()) {
            if (auto r = otype_cmap_parse_map(map_bytes); not r.empty()) {
                return r;
            }
        }
    }

    throw parse_error("'cmap' no compatible character map found.");
}

}} // namespace hi::v1
