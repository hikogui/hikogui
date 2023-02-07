// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "otype_utilities.hpp"
#include "../utility/module.hpp"
#include "../i18n/language_tag.hpp"
#include <span>
#include <cstddef>

namespace hi { inline namespace v1 {
namespace detail {

struct otype_name_language_entry_type {
    big_uint16_buf_t length;
    big_uint16_buf_t offset;
};

[[nodiscard]] inline language_tag otype_name_get_language_tag(
    std::span<std::byte const> storage_bytes,
    std::span<otype_name_language_entry_type const> language_tag_table,
    uint16_t language_id)
{
    hi_axiom(language_id >= 0x8000);
    hilet& entry = hi_check_at(language_tag_table, language_id - 0x8000);
    hilet tag_bytes = hi_check_subspan(storage_bytes, *entry.offset, *entry.length);
    return language_tag{std::string_view{reinterpret_cast<char const *>(tag_bytes.data()), tag_bytes.size()}};
}

[[nodiscard]] inline language_tag otype_name_get_language_unicode(
    std::span<std::byte const> storage_bytes,
    std::span<otype_name_language_entry_type const> language_tag_table,
    uint16_t language_id)
{
    if (language_id == 0 or language_id == 0xffff) {
        // In Unicode mode only language_id zero is defined, and defined as "no particular language".
        return {"en"};

    } else if (language_id >= 0x8000) {
        // Use the optional language tag table.
        return otype_name_get_language_tag(storage_bytes, language_tag_table, language_id);

    } else {
        // This language id is invalid.
        return {};
    }
}

[[nodiscard]] inline language_tag
otype_name_get_language_quickdraw(std::span<std::byte const> storage_bytes, uint16_t platform_specific_id, uint16_t language_id)
{
    if (platform_specific_id == 0 and language_id == 0) {
        // Roman, English.
        return language_tag{"en"};
    } else {
        return language_tag{};
    }
}

[[nodiscard]] inline language_tag otype_name_get_language_microsoft(
    std::span<std::byte const> storage_bytes,
    std::span<otype_name_language_entry_type const> language_tag_table,
    uint16_t language_id)
{
    if (language_id == 0x409) {
        return language_tag{"en-US"};
    } else if (language_id >= 0x8000) {
        return otype_name_get_language_tag(storage_bytes, language_tag_table, language_id);
    } else {
        return language_tag{};
    }
}

[[nodiscard]] inline language_tag otype_name_get_language(
    std::span<std::byte const> storage_bytes,
    std::span<otype_name_language_entry_type const> language_tag_table,
    uint16_t platform_id,
    uint16_t platform_specific_id,
    uint16_t language_id)
{
    switch (platform_id) {
    case 0:
        return otype_name_get_language_unicode(storage_bytes, language_tag_table, language_id);
    case 1:
        return otype_name_get_language_quickdraw(storage_bytes, platform_specific_id, language_id);
    case 3:
        return otype_name_get_language_microsoft(storage_bytes, language_tag_table, language_id);
    default:
        return {};
    }
}

} // namespace detail

/** Get a name from the name table.
 *
 * @param bytes The bytes of the name table.
 * @param name_id The name to find a string for.
 * @param language The language to find the string for (default "en").
 * @return The string of the name was found, or std::nullopt if not found.
 */
[[nodiscard]] inline std::optional<std::string>
otype_name_search(std::span<std::byte const> bytes, uint16_t name_id, language_tag language = language_tag{"en"})
{
    struct header_type_0 {
        big_uint16_buf_t format;
        big_uint16_buf_t count;
        big_uint16_buf_t storage_offset;
    };

    struct header_type_1 : header_type_0 {
        big_uint16_buf_t language_tag_count;
    };

    struct entry_type {
        big_uint16_buf_t platform_id;
        big_uint16_buf_t platform_specific_id;
        big_uint16_buf_t language_id;
        big_uint16_buf_t name_id;
        big_uint16_buf_t length;
        big_uint16_buf_t offset;
    };

    auto offset = 0_uz;
    hilet& header = implicit_cast<header_type_0>(offset, bytes);

    hilet format = *header.format;
    hi_check(format == 0 or format == 1, "'name' table must be format 0 or format 1.");

    hilet storage_bytes = hi_check_subspan(bytes, *header.storage_offset);

    auto language_tag_count = 0_uz;
    if (format == 1) {
        offset = 0_uz;
        hilet header1 = implicit_cast<header_type_1>(offset, bytes);
        language_tag_count = *header1.language_tag_count;
    }

    hilet language_tag_table = implicit_cast<detail::otype_name_language_entry_type>(offset, bytes, language_tag_count);

    hilet entries = implicit_cast<entry_type>(offset, bytes, *header.count);
    for (hilet& entry : entries) {
        if (*entry.name_id != name_id) {
            continue;
        }

        hilet platform_id = *entry.platform_id;
        hilet platform_specific_id = *entry.platform_specific_id;

        hilet name_language = detail::otype_name_get_language(
            storage_bytes, language_tag_table, platform_id, platform_specific_id, *entry.language_id);

        if (not match(name_language, language)) {
            continue;
        }

        hilet name_bytes = hi_check_subspan(storage_bytes, *entry.offset, *entry.length);

        if (auto s = otype_get_string(name_bytes, platform_id, platform_specific_id)) {
            return s;
        }
    }

    return std::nullopt;
}

[[nodiscard]] inline auto otype_name_get_family(std::span<std::byte const> bytes)
{
    struct return_type {
        std::string family_name;
        std::string sub_family_name;
    };

    auto r = return_type{};

    if (auto typographic_family_name = otype_name_search(bytes, 16)) {
        r.family_name = std::move(*typographic_family_name);
    } else if (auto common_family_name = otype_name_search(bytes, 1)) {
        r.family_name = std::move(*common_family_name);
    }

    if (auto typographic_sub_family_name = otype_name_search(bytes, 17)) {
        r.sub_family_name = std::move(*typographic_sub_family_name);
    } else if (auto common_sub_family_name = otype_name_search(bytes, 2)) {
        r.sub_family_name = std::move(*common_sub_family_name);
    }

    return r;
}

}} // namespace hi::v1
