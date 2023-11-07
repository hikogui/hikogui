// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <span>
#include <cstddef>

export module hikogui_font_otype_sfnt;
import hikogui_font_otype_utilities;
import hikogui_utility;

export namespace hi {
inline namespace v1 {

template<fixed_string Name>
[[nodiscard]] std::span<std::byte const> otype_sfnt_search(std::span<std::byte const> bytes)
{
    struct header_type {
        big_uint32_buf_t scaler_type;
        big_uint16_buf_t num_tables;
        big_uint16_buf_t search_range;
        big_uint16_buf_t entry_selector;
        big_uint16_buf_t range_shift;
    };

    struct entry_type {
        big_uint32_buf_t tag;
        big_uint32_buf_t check_sum;
        big_uint32_buf_t offset;
        big_uint32_buf_t length;
    };

    std::size_t offset = 0;
    hilet& header = implicit_cast<header_type>(offset, bytes);

    if (not (*header.scaler_type == "true"_fcc or *header.scaler_type == 0x00010000)) {
        throw parse_error("sfnt.scalerType is not 'true' or 0x00010000");
    }

    hilet entries = implicit_cast<entry_type>(offset, bytes, *header.num_tables);

    if (hilet entry = fast_binary_search_eq<std::endian::big>(entries, fourcc<Name>())) {
        return hi_check_subspan(bytes, *entry->offset, *entry->length);
    } else {
        return {};
    }
}

}}
