// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "phrasing.hpp"
#include "../i18n/i18n.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.unicode : grapheme_attributes);

hi_export namespace hi {
inline namespace v1 {

struct grapheme_attributes {
    iso_639 language;
    iso_15924 script;
    iso_3166 region;
    hi::phrasing phrasing;

    constexpr grapheme_attributes() noexcept = default;
    constexpr grapheme_attributes(grapheme_attributes const&) noexcept = default;
    constexpr grapheme_attributes(grapheme_attributes&&) noexcept = default;
    constexpr grapheme_attributes& operator=(grapheme_attributes const&) noexcept = default;
    constexpr grapheme_attributes& operator=(grapheme_attributes&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(grapheme_attributes const&, grapheme_attributes const&) noexcept = default;

    constexpr grapheme_attributes(hi::phrasing phrasing) noexcept : language(), script(), region(), phrasing(phrasing) {}
};

struct grapheme_attribute_mask {
    iso_639 language;
    iso_15924 script;
    iso_3166 region;
    hi::phrasing_mask phrasing = hi::phrasing_mask::all;

    constexpr grapheme_attribute_mask() noexcept = default;
    constexpr grapheme_attribute_mask(grapheme_attribute_mask const&) noexcept = default;
    constexpr grapheme_attribute_mask(grapheme_attribute_mask&&) noexcept = default;
    constexpr grapheme_attribute_mask& operator=(grapheme_attribute_mask const&) noexcept = default;
    constexpr grapheme_attribute_mask& operator=(grapheme_attribute_mask&&) noexcept = default;
    [[nodiscard]] constexpr friend bool
    operator==(grapheme_attribute_mask const&, grapheme_attribute_mask const&) noexcept = default;

    constexpr grapheme_attribute_mask(hi::phrasing_mask phrasing) noexcept : language(), script(), region(), phrasing(phrasing) {}

    [[nodiscard]] constexpr friend bool
    matches(grapheme_attribute_mask const& haystack, grapheme_attributes const& needle) noexcept
    {
        return matches(haystack.language, needle.language) and matches(haystack.script, needle.script) and
            matches(haystack.region, needle.region) and matches(haystack.phrasing, needle.phrasing);
    }
};

} // namespace v1
}
