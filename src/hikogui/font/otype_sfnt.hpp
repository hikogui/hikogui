// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "otype_utilities.hpp"
#include "../utility/module.hpp"
#include <span>
#include <cstddef>

namespace hi {
inline namespace v1 {

template<fixed_string Name>
[[nodiscard]] std::span<std::byte const> otype_search_sfnt(std::span<std::byte const> data)
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
    hilet header = make_placement_ptr<header_type>(data, offset);

    if (not (*header->scaler_type == "true"_fcc or *header->scaler_type == 0x00010000)) {
        throw parse_error("sfnt.scalerType is not 'true' or 0x00010000");
    }

    hilet entries = make_placement_array<entry_type>(data, offset, *header->num_tables);

    if (auto entry = otype_search_table(entries, fourcc<Name>())) {
        return data.subspan(*entry->offset, *entry->length);
    } else {
        return {};
    }
}

}}
