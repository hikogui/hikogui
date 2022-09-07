// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font_variant.hpp"
#include "text_decoration.hpp"
#include "font_family_id.hpp"
#include "text_phrasing.hpp"
#include "semantic_text_style.hpp"
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
    ::hi::color color;
    float size;
    font_variant variant;
    text_decoration decoration;

    text_sub_style() noexcept = default;

    text_sub_style(
        text_phrasing_mask phrasing_mask,
        iso_639 language_filter,
        iso_15924 script_filter,
        font_family_id family_id,
        font_variant variant,
        float size,
        ::hi::color color,
        text_decoration decoration) noexcept :
        phrasing_mask(phrasing_mask),
        language_filter(language_filter),
        script_filter(script_filter),
        family_id(family_id),
        color(color),
        size(size),
        variant(variant),
        decoration(decoration)
    {
    }

    [[nodiscard]] size_t hash() const noexcept
    {
        auto r = 0_uz;
        r ^= std::hash<text_phrasing_mask>{}(phrasing_mask);
        r ^= std::hash<iso_639>{}(language_filter);
        r ^= std::hash<iso_15924>{}(script_filter);
        r ^= std::hash<font_family_id>{}(family_id);
        r ^= std::hash<hi::color>{}(color);
        r ^= std::hash<float>{}(size);
        r ^= std::hash<font_variant>{}(variant);
        r ^= std::hash<text_decoration>{}(decoration);
        return r;
    }

    [[nodiscard]] float cap_height(font_book const& font_book) const noexcept;

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

    [[nodiscard]] friend bool operator==(text_sub_style const&, text_sub_style const&) noexcept = default;
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::text_sub_style> {
    [[nodiscard]] size_t operator()(hi::text_sub_style const& rhs) const noexcept
    {
        return rhs.hash();
    }
};

namespace hi::inline v1::detail {

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

    [[nodiscard]] size_t hash() const noexcept
    {
        auto r = 0_uz;
        for (hilet& sub_style : _sub_styles) {
            r ^= std::hash<text_sub_style>{}(sub_style);
        }
        return r;
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
};

} // namespace hi::inline v1::detail

template<>
struct std::hash<hi::detail::text_style_impl> {
    [[nodiscard]] size_t operator()(hi::detail::text_style_impl const& rhs) const noexcept
    {
        return rhs.hash();
    }
};

namespace hi::inline v1 {
namespace detail {
inline auto text_styles = stable_set<text_style_impl>{};
}

class text_style {
public:
    using int_type = uint16_t;

    constexpr text_style() : _value(0xffff) {}

    constexpr text_style(semantic_text_style rhs) noexcept : _value(0xff00 + to_underlying(rhs)) {}

    text_style(std::vector<text_sub_style> rhs) noexcept
    {
        hilet index = detail::text_styles.emplace(std::move(rhs));
        if (index < 0xff00) {
            _value = narrow_cast<uint16_t>(index);
        } else {
            hi_log_error_once("text-style:error:too-many", "Too many text-styles");
            // semantic text-style "label".
            _value = 0xff00;
        }
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _value == 0xffff;
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr bool is_semantic() const noexcept
    {
        hi_axiom(not empty());
        return _value >= 0xff00;
    }

    constexpr explicit operator semantic_text_style() const noexcept
    {
        return static_cast<semantic_text_style>(narrow_cast<std::underlying_type_t<semantic_text_style>>(_value - 0xff00));
    }

    text_sub_style const *operator->() const noexcept
    {
        hi_axiom(not empty());
        if (_value < 0xff00) {
            return std::addressof(detail::text_styles[_value].back());
        } else {
            hi_not_implemented();
        }
    }

    text_sub_style const& operator*() const noexcept
    {
        hi_axiom(not empty());
        if (_value < 0xff00) {
            return detail::text_styles[_value].back();
        } else {
            hi_not_implemented();
        }
    }

    text_sub_style const& sub_style(text_phrasing phrasing, iso_639 language, iso_15924 script) const noexcept
    {
        for (hilet& style : detail::text_styles[_value]) {
            if (style.matches(phrasing, language, script)) {
                return style;
            }
        }
    }

private:
    int_type _value;
};

} // namespace hi::inline v1
