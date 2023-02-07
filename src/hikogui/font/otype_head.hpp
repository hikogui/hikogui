// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "otype_utilities.hpp"
#include "../utility/module.hpp"
#include <span>
#include <cstddef>

namespace hi { inline namespace v1 {

[[nodiscard]] inline auto otype_head_parse(std::span<std::byte const> bytes)
{
    struct header_type {
        big_uint16_buf_t major_version;
        big_uint16_buf_t minor_version;
        otype_fixed15_16_buf_t font_revision;
        big_uint32_buf_t check_sum_adjustment;
        big_uint32_buf_t magic_number;
        big_uint16_buf_t flags;
        big_uint16_buf_t units_per_em;
        big_uint64_buf_t created;
        big_uint64_buf_t modified;
        otype_fword_buf_t x_min;
        otype_fword_buf_t y_min;
        otype_fword_buf_t x_max;
        otype_fword_buf_t y_max;
        big_uint16_buf_t mac_style;
        big_uint16_buf_t lowest_rec_PPEM;
        big_int16_buf_t font_direction_hint;
        big_int16_buf_t index_to_loc_format;
        big_int16_buf_t glyph_data_format;
    };

    struct return_type {
        bool loca_is_offset32;
        float em_scale;
    };

    hilet& header = implicit_cast<header_type>(bytes);

    hi_check(*header.major_version == 1 and *header.minor_version == 0, "'head' version is not 1.0");
    hi_check(*header.magic_number == 0x5f0f3cf5, "'head' magic is not 0x5f0f3cf5");

    // All data to be returned must be copied before it is checked, because the
    // underlying data may be modified by an external application.
    auto r = return_type{};

    switch (*header.index_to_loc_format) {
    case 0:
        r.loca_is_offset32 = false;
        break;
    case 1:
        r.loca_is_offset32 = true;
        break;
    default:
        throw parse_error("'head' index_to_loc_format must be 0 or 1.");
    }

    if (hilet units_per_em = *header.units_per_em; units_per_em != 0.0) {
        r.em_scale = 1.0f / units_per_em;
    } else {
        throw parse_error("'head' units_per_em must not be 0.0.");
    }

    return r;
}

}} // namespace hi::v1
