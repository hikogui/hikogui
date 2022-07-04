// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font_variant.hpp"
#include "text_decoration.hpp"
#include "font_family_id.hpp"
#include "text_phrasing.hpp"
#include "../color/color.hpp"
#include "../i18n/iso_15924.hpp"
#include "../i18n/iso_639.hpp"
#include "../numbers.hpp"
#include "../log.hpp"
#include "../stable_set.hpp"
#include "../cast.hpp"
#include "../assert.hpp"
#include <ostream>
#include <vector>
#include <algorithm>

namespace hi::inline v1 {
class font_book;

struct text_sub_style {
    text_phrasing_mask phrasing_mask;
    iso_639 language_filter;
    iso_15924 script_filter;

    font_family_id family_id;
    color color;
    float size;
    font_variant variant;
    text_decoration decoration;

    text_sub_style() noexcept = default;

    [[nodiscard]] friend bool operator==(text_sub_style const&, text_sub_style const&) noexcept = default;
    [[nodiscard]] friend auto operator<=>(text_sub_style const&, text_sub_style const&) noexcept = default;

    [[nodiscard]] bool matches(text_phrasing phrasing, iso_639 language, iso_15924 script) const noexcept
    {
        if (not to_bool(phrasing_mask & to_text_phrasing_mask(phrasing))) {
            return false;
        }
        if (language_filter and language and language_filter != language) {
            return false;
        }
        if (script_filter and script and script_filter != script) {
            return false;
        }
        return true;
    }
};

struct text_style_impl {
    using value_type = text_sub_style;
    using reference = value_type const&;
    using vector_type = std::vector<value_type>;
    using iterator = vector_type::const_iterator;

    vector_type _sub_styles;

    constexpr text_style_impl() noexcept = default;

    text_style_impl(std::vector<text_sub_style> sub_styles) noexcept : _sub_styles(std::move(sub_styles))
    {
        hi_axiom(not empty());
        hi_axiom(all(back().phrasing_mask));
        hi_axiom(back().language_filter.empty());
        hi_axiom(back().script_filter.empty());
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return _sub_styles.empty();
    }

    explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] reference back() const noexcept
    {
        return _sub_styles.back();
    }

    [[nodiscard]] iterator begin() const noexcept
    {
        return _sub_styles.begin();
    }

    [[nodiscard]] iterator end() const noexcept
    {
        return _sub_styles.end();
    }

    [[nodiscard]] constexpr friend bool operator==(text_style_impl const&, text_style_impl const&) noexcept = default;

    [[nodiscard]] constexpr friend auto operator<=>(text_style_impl const& lhs, text_style_impl const& rhs) noexcept
    {
        return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
};

inline auto text_style_impls = stable_set<text_style_impl>{};

class text_style {
public:
    using int_type = uint16_t;

    text_style() : _value(0xffff) {}

    text_style(semantic_text_style rhs) noexcept : _value(0xff00 + to_underlying(rhs)) {}

    text_style(std::vector<text_sub_style> rhs) noexcept
    {
        hilet index = text_style_impls.emplace(std::move(rhs));
        if (index < 0xff00) {
            _value = narrow_cast<uint16_t>(index);
        } else {
            hi_log_error_once("text-style:error:too-many", "Too many text-styles");
            // semantic text-style "label".
            _value = 0xff00;
        }
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return _value == 0xffff;
    }

    explicit operator bool() const noexcept
    {
        return not empty();
    }

    text_sub_style const *operator->() const noexcept
    {
        hi_axiom(not empty());
        if (_value < 0xff00) {
            return std::addressof(text_style_impls[_value].back());
        } else {
            hi_not_implemented();
        }
    }

    text_sub_style const& operator*() const noexcept
    {
        hi_axiom(not empty());
        if (_value < 0xff00) {
            return text_style_impls[_value].back();
        } else {
            hi_not_implemented();
        }
    }

private:
    int_type _value;
};

} // namespace hi::inline v1
