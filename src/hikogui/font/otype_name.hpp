// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "otype_utilities.hpp"
#include "../utility/module.hpp"
#include <span>
#include <cstddef>

namespace hi { inline namespace v1 {


[[nodiscard]] auto otype_parse_name(std::span<std::byte const> bytes)
{
    struct header_type {
        big_uint16_buf_t format;
        big_uint16_buf_t count;
        big_uint16_buf_t string_offset;
    };

    struct entry_type {
        big_uint16_buf_t platform_id;
        big_uint16_buf_t platform_specific_id;
        big_uint16_buf_t language_id;
        big_uint16_buf_t name_id;
        big_uint16_buf_t length;
        big_uint16_buf_t offset;
    };

    struct return_type {
        std::string family_name;
        std::string sub_family_name;
    };

    auto offset = 0_uz;
    hilet& header = implicit_cast<header_type>(offset, bytes);

    hi_parse_check(*header.format == 0 || *header.format == 1, "Name table format must be 0 or 1");
    std::size_t storage_area_offset = *header.string_offset;

    hilet entries = implicit_cast<entry_type>(offset, bytes, *header.count);

    auto has_typographic_family = false;
    auto has_typographic_sub_family = false;
    auto r = return_type{};
    for (hilet& entry : entries) {
        hilet name_offset = storage_area_offset + *entry.offset;
        hilet name_size = *entry.length;

        hi_parse_check(name_offset + name_size <= bytes.size(), "Requesting name at offset beyond name table");
        hilet name_bytes = bytes.subspan(name_offset, name_size);

        hilet language_id = *entry.language_id;
        hilet platform_id = *entry.platform_id;
        hilet platform_specific_id = *entry.platform_specific_id;

        switch (*entry.name_id) {
        case 1:
            // font family. But typographic-family has priority.
            if (not has_typographic_family) {
                if (auto s = otype_get_string(name_bytes, platform_id, platform_specific_id, language_id)) {
                    r.family_name = std::move(*s);
                }
            }
            break;

        case 2:
            // font sub-family. But typographic-sub-family has priority.
            if (not has_typographic_sub_family) {
                if (auto s = otype_get_string(name_bytes, platform_id, platform_specific_id, language_id)) {
                    r.sub_family_name = std::move(*s);
                }
            }
            break;

        case 16:
            // Typographic family.
            if (auto s = otype_get_string(name_bytes, platform_id, platform_specific_id, language_id)) {
                r.family_name = std::move(*s);
                has_typographic_family = true;
            }
            break;

        case 17:
            // Typographic sub-family.
            if (auto s = otype_get_string(name_bytes, platform_id, platform_specific_id, language_id)) {
                r.sub_family_name = std::move(*s);
                has_typographic_sub_family = true;
            }
            break;

        default:
            continue;
        }
    }

    return r;
}

}} // namespace hi::v1
