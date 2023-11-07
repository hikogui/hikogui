// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "otype_utilities.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <span>
#include <cstddef>

hi_export_module(hikogui.font.maxp);

hi_export namespace hi { inline namespace v1 {

[[nodiscard]] hi_inline auto otype_maxp_parse(std::span<std::byte const> bytes)
{
    struct header_type_05 {
        big_uint32_buf_t version;
        big_uint16_buf_t num_glyphs;
    };

    struct header_type_10 {
        big_uint32_buf_t version;
        big_uint16_buf_t num_glyphs;
        big_uint16_buf_t max_points;
        big_uint16_buf_t max_contours;
        big_uint16_buf_t max_component_points;
        big_uint16_buf_t max_component_contours;
        big_uint16_buf_t max_zones;
        big_uint16_buf_t max_twilight_points;
        big_uint16_buf_t max_storage;
        big_uint16_buf_t max_function_defs;
        big_uint16_buf_t max_instruction_defs;
        big_uint16_buf_t max_stack_elements;
        big_uint16_buf_t max_size_of_instructions;
        big_uint16_buf_t max_component_elements;
        big_uint16_buf_t max_component_depth;
    };

    struct return_type {
        uint16_t num_glyphs;
    };

    hilet& header = implicit_cast<header_type_05>(bytes);
    hilet version = *header.version;
    hi_check(version == 0x00010000 || version == 0x00005000, "MAXP version must be 0.5 or 1.0");

    auto r = return_type{};
    r.num_glyphs = *header.num_glyphs;
    return r;
}

}} // namespace hi::v1
